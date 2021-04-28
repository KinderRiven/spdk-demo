/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-28 17:42:06
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
    uint64_t thread_id;
    uint32_t io_depth;
    size_t block_size;
    std::queue<struct spdk_poller*> q_poller;
};

spdk_thread_context_t g_spdk_ctx[128];

int poller_bdev_read(void* argv)
{
}

int poller_bdev_write(void* argv)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)argv;
    int _rc = spdk_bdev_write(_ctx->desc, _ctx->channel, _ctx->dma_buf, 0, _ctx->block_size, nullptr, nullptr);
    assert(_rc == 0);
}

int poller_clean_cq(void* argv)
{
    spdk_thread_context_t* _ctx = (spdk_thread_context_t*)argv;
}

void start_io_event(void* bdev, void* desc)
{
    struct spdk_bdev* _bdev = (struct spdk_bdev*)bdev;
    struct spdk_bdev_desc* _desc = (struct spdk_bdev_desc*)desc;

    int _core_id = spdk_env_get_current_core();
    printf("Fuck you, man! start_event [core_%d].\n", _core_id);

    // create thread for each core
    char _name[128];
    sprintf(_name, "%d", _core_id);
    struct spdk_thread* _thread = spdk_thread_create(_name, nullptr);
    spdk_set_thread(_thread);

    uint32_t _thread_id = spdk_thread_get_id(spdk_get_thread());
    g_spdk_ctx[_thread_id].thread_id = _thread_id;
    g_spdk_ctx[_thread_id].bdev = _bdev;
    g_spdk_ctx[_thread_id].desc = _desc;
    g_spdk_ctx[_thread_id].channel = spdk_bdev_get_io_channel(_desc);
    g_spdk_ctx[_thread_id].dma_buf = spdk_dma_zmalloc(g_spdk_ctx->block_size, 4096UL, nullptr);

    if (g_spdk_ctx[_thread_id].channel == nullptr) {
        printf("spdk_bdev_get_io_channe failed!\n");
        exit(1);
    }
    if (g_spdk_ctx[_thread_id].dma_buf == nullptr) {
        printf("spdk_dma_zmalloc failed!\n");
        exit(1);
    }

    {
        printf("polling_poller_register [core_id:%d]!\n", _core_id);
        struct spdk_poller* _poller = spdk_poller_register(poller_bdev_write, (void*)&g_spdk_ctx[_thread_id], 0);
        assert(_poller != nullptr);
        g_spdk_ctx[_thread_id].q_poller.push(_poller);
    }
    {
        printf("polling_poller_register [core_id:%d]!\n", _core_id);
        struct spdk_poller* _poller = spdk_poller_register(poller_clean_cq, (void*)&g_spdk_ctx[_thread_id], 0);
        assert(_poller != nullptr);
        g_spdk_ctx[_thread_id].q_poller.push(_poller);
    }
}

void stop_event(void* arg1, void* arg2)
{
    int _thread_id = spdk_thread_get_id(spdk_get_thread());
    printf("Fuck you, man! stop_event [thread%d/core%d]\n", _thread_id, spdk_env_get_current_core());

    // 每个线程退出的时候必须注销自己注册过的poller
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
    struct spdk_bdev* _bdev;
    struct spdk_bdev_desc* _desc;

    _bdev = spdk_bdev_get_by_name("Nvme0");
    _bdev = spdk_bdev_first();
    if (_bdev == nullptr) {
        printf("spdk_bdev_get_by_name failed! [Nvme0]\n");
        exit(1);
    }

    _rc = spdk_bdev_open(_bdev, true, nullptr, nullptr, &_desc);
    if (_rc) {
        printf("spdk_bdev_open failed!\n");
        exit(1);
    } else {
        printf("spdk_bdev_open [bs:%zu][align:%zu]\n", spdk_bdev_get_block_size(_bdev), spdk_bdev_get_buf_align(_bdev));
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