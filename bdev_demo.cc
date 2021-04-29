/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-29 14:42:06
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /spdk-demo/reactor_demo.cc
 */
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <queue>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/thread.h"

struct spdk_thread_context_t {
public:
    void* dma_buf;
    struct spdk_bdev* bdev;
    struct spdk_bdev_desc* desc;
    struct spdk_io_channel* channel;

public:
    uint32_t io_cnt = 0;

public:
    uint64_t thread_id;
    uint32_t io_depth;
    size_t block_size;
    std::queue<struct spdk_poller*> q_poller;
};

spdk_thread_context_t g_spdk_ctx[128];

static void io_cb(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)cb_arg;
    _ctx->io_cnt++;
}

static void bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev* bdev, void* event_ctx)
{
    printf("bdev_event_cb [%d]\n", type);
}

int poller_bdev_read(void* argv)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)argv;
    int _rc = spdk_bdev_read(_ctx->desc, _ctx->channel, _ctx->dma_buf, 0, _ctx->block_size, nullptr, nullptr);
    assert(_rc == 0);
}

int poller_bdev_write(void* argv)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)argv;
    int _rc = spdk_bdev_write(_ctx->desc, _ctx->channel, _ctx->dma_buf, 0, _ctx->block_size, io_cb, argv);
    assert(_rc == 0);
}

int poller_clean_cq(void* argv)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)argv;
}

void start_io_event(void* bdev, void* desc)
{
#if 0
    struct spdk_bdev* _bdev = (struct spdk_bdev*)bdev;
    struct spdk_bdev_desc* _desc = (struct spdk_bdev_desc*)desc;

    int _core_id = spdk_env_get_current_core();
    printf("Fuck you, man! start_event [core_%d].\n", _core_id);

    char _name[128];
    sprintf(_name, "%d", _core_id);
    struct spdk_thread* _thread = spdk_thread_create(_name, nullptr);
    spdk_set_thread(_thread);

    uint32_t _thread_id = spdk_thread_get_id(spdk_get_thread());
    g_spdk_ctx[_thread_id].thread_id = _thread_id;
    g_spdk_ctx[_thread_id].bdev = _bdev;
    g_spdk_ctx[_thread_id].desc = _desc;
    g_spdk_ctx[_thread_id].block_size = 4096UL;

    assert(_desc != nullptr);
    g_spdk_ctx[_thread_id].channel = spdk_bdev_get_io_channel(_desc);
    if (g_spdk_ctx[_thread_id].channel == nullptr) {
        spdk_bdev_close(_desc);
        SPDK_ERRLOG("spdk_bdev_get_io_channe failed.\n");
        exit(1);
    }

    g_spdk_ctx[_thread_id].dma_buf = spdk_dma_zmalloc(g_spdk_ctx->block_size, 4096UL, nullptr);
    if (g_spdk_ctx[_thread_id].dma_buf == nullptr) {
        printf("spdk_dma_zmalloc failed!\n");
        exit(1);
    }

    printf("polling_poller_register [poller_bdev_write][core_id:%d]!\n", _core_id);
    struct spdk_poller* _poller = spdk_poller_register(poller_bdev_write, (void*)&g_spdk_ctx[_thread_id], 0);
    assert(_poller != nullptr);
    g_spdk_ctx[_thread_id].q_poller.push(_poller);

    printf("polling_poller_register [poller_clean_cq][core_id:%d]!\n", _core_id);
    _poller = spdk_poller_register(poller_clean_cq, (void*)&g_spdk_ctx[_thread_id], 0);
    assert(_poller != nullptr);
    g_spdk_ctx[_thread_id].q_poller.push(_poller);
#endif
}

void stop_event(void* arg1, void* arg2)
{
    int _thread_id = spdk_thread_get_id(spdk_get_thread());
    printf("Fuck you, man! stop_event [thread%d/core%d][io_cnt:%d]\n",
        _thread_id, spdk_env_get_current_core(), g_spdk_ctx[_thread_id].io_cnt);

    while (!g_spdk_ctx[_thread_id].q_poller.empty()) {
        struct spdk_poller* _poller = g_spdk_ctx[_thread_id].q_poller.front();
        g_spdk_ctx[_thread_id].q_poller.pop();
        spdk_poller_unregister(&_poller);
    }

    struct spdk_thread* _thread = spdk_get_thread();
    spdk_thread_exit(_thread);
}

void start_app(void* cb)
{
    int _rc;
    struct spdk_bdev* _bdev = nullptr;
    struct spdk_bdev_desc* _desc = nullptr;
    struct spdk_io_channel* _io_channel = nullptr;

    struct spdk_bdev_opts __opts;
    spdk_bdev_get_opts(&__opts, sizeof(__opts));
    printf("bdev_pool_size:%d\n", __opts.bdev_io_pool_size);

    _bdev = spdk_bdev_first();
    while (_bdev != nullptr) {
        printf("module_name [%s]\n", spdk_bdev_get_module_name(_bdev));
        _bdev = spdk_bdev_next(_bdev);
    }

    _bdev = spdk_bdev_get_by_name("Nvme0n1");
    if (_bdev == nullptr) {
        printf("spdk_bdev_get_by_name failed! [Nvme0n1]\n");
        exit(1);
    } else {
        assert(_bdev != nullptr);
        _rc = spdk_bdev_open_ext("Nvme0n1", true, bdev_event_cb, nullptr, &_desc);
        _bdev = spdk_bdev_desc_get_bdev(_desc);
        printf("spdk_bdev_open [nvme][bs:%zu][align:%zu]\n", spdk_bdev_get_block_size(_bdev), spdk_bdev_get_buf_align(_bdev));
        if (_rc) {
            printf("spdk_bdev_open_ext failed!\n");
            exit(1);
        } else {
            assert(_desc != nullptr);
            printf("spdk_bdev_open_ext ok!\n");
            _io_channel = spdk_bdev_get_io_channel(_desc);
            if (_io_channel != nullptr) {
                printf("spdk_bdev_get_io_channel ok!\n");
            } else {
                printf("spdk_bdev_get_io_channel failed!\n");
                exit(1);
            }
        }
    }

    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("[START APPLICATION!][core_count:%d/%d]\n", spdk_env_get_current_core(), spdk_env_get_core_count());
    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, start_io_event, (void*)_bdev, (void*)_desc);
            spdk_event_call(event);
        }
    }
}

void stop_app()
{
    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("[STOP APPLICATION!][core_count:%d/%d]\n", spdk_env_get_current_core(), spdk_env_get_core_count());

    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, stop_event, nullptr, nullptr);
            spdk_event_call(event);
        }
    }

    struct spdk_bdev_desc* _desc = g_spdk_ctx[0].desc;
    spdk_bdev_close(_desc);
    spdk_app_stop(0);
}

int main(int argc, char** argv)
{
    int _rc;
    struct spdk_app_opts _app_opts = {};

    // 1.参数化参数
    spdk_app_opts_init(&_app_opts, sizeof(_app_opts));
    _app_opts.name = "FuckYouMan";
    _app_opts.shutdown_cb = stop_app;

    // 2.参数解析
    if ((_rc = spdk_app_parse_args(argc, argv, &_app_opts, nullptr, nullptr, nullptr, nullptr)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(_rc);
    }

    printf("OPT [name:%s][file_name:%s][reactor_mask:%s][main_core:%d]\n",
        _app_opts.name, _app_opts.json_config_file, _app_opts.reactor_mask, _app_opts.main_core);
    _rc = spdk_app_start(&_app_opts, start_app, nullptr);
    spdk_app_fini();
    return 0;
}