/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-29 17:14:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /spdk-demo/reactor_demo.cc
 */
#include <assert.h>
#include <atomic>
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
#include "spdk/stdinc.h"
#include "spdk/thread.h"

struct spdk_core_context_t {
public:
    uint32_t core_id;
    struct spdk_bdev* bdev;
    struct spdk_bdev_desc* desc;
    struct spdk_io_channel* channel;

public:
    uint32_t io_cnt = 0;

public:
    uint32_t io_depth;
    size_t block_size;

public:
    struct spdk_thread* thread;
    std::queue<struct spdk_poller*> q_poller;
};

static int g_num_run_thread = 0;
static int g_app_rc;
static char g_bdev_name[] = "Nvme0n1";
static struct spdk_bdev* g_bdev = nullptr;
static struct spdk_bdev_desc* g_desc = nullptr;
static spdk_core_context_t g_spdk_ctx[128];

static void io_cb(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    spdk_bdev_free_io(bdev_io);
    spdk_dma_free(cb_arg);
}

static void bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev* bdev, void* event_ctx)
{
    printf("bdev_event_cb [%d]\n", type);
}

int poller_bdev_read(void* argv)
{
    spdk_core_context_t* _ctx = (spdk_core_context_t*)argv;
    void* _wbuf = spdk_dma_zmalloc(_ctx->block_size, 4096UL, nullptr);
    _ctx->io_cnt++;
    // printf("%d\n", _ctx->io_cnt);
    int _rc = spdk_bdev_read(_ctx->desc, _ctx->channel, _wbuf, 0, _ctx->block_size, io_cb, _wbuf);
    if (_rc) {
        printf("spdk_bdev_read failed, %d", _rc);
    }
    assert(_rc == 0);
}

int poller_bdev_write(void* argv)
{
    spdk_core_context_t* _ctx = (spdk_core_context_t*)argv;
    void* _wbuf = spdk_dma_zmalloc(_ctx->block_size, 4096UL, nullptr);
    _ctx->io_cnt++;
    // printf("%d\n", _ctx->io_cnt);
    int _rc = spdk_bdev_write(_ctx->desc, _ctx->channel, _wbuf, 0, _ctx->block_size, io_cb, _wbuf);
    if (_rc) {
        printf("spdk_bdev_write failed, %d", _rc);
    }
    assert(_rc == 0);
}

int poller_clean_cq(void* argv)
{
    struct spdk_bdev_io_wait_entry bdev_io_wait;
    spdk_core_context_t* _ctx = (spdk_core_context_t*)argv;
}

void start_io_event(void* bdev, void* desc)
{
    g_num_run_thread++;

    int _core_id = spdk_env_get_current_core();
    struct spdk_bdev* _bdev = (struct spdk_bdev*)bdev;
    struct spdk_bdev_desc* _desc = (struct spdk_bdev_desc*)desc;

#if 0
    char _s_cpu_mask[128];
    struct spdk_cpuset* _cpu_mask = spdk_cpuset_alloc();
    spdk_cpuset_set_cpu(_cpu_mask, _core_id, true);
    spdk_thread_set_cpumask(_cpu_mask);
    spdk_cpuset_parse(_cpu_mask, _s_cpu_mask);

    char _name[128];
    sprintf(_name, "%d", _core_id);
    struct spdk_thread* _thread = spdk_thread_create(_name, _cpu_mask);
    spdk_set_thread(_thread);
    int _thread_id = spdk_thread_get_id(spdk_get_thread());
#endif

    struct spdk_thread* _thread = spdk_get_thread();
    spdk_set_thread(_thread);
    int _thread_id = spdk_thread_get_id(spdk_get_thread());

    printf("Fuck you, man! start_io_event [thread_id:%d][core_id:%d].\n", _thread_id, _core_id);
    g_spdk_ctx[_core_id].thread = _thread;
    g_spdk_ctx[_core_id].core_id = _core_id;
    g_spdk_ctx[_core_id].bdev = _bdev;
    g_spdk_ctx[_core_id].desc = _desc;
    g_spdk_ctx[_core_id].block_size = 4096UL;

    assert(_desc != nullptr);
    g_spdk_ctx[_core_id].channel = spdk_bdev_get_io_channel(_desc);
    if (g_spdk_ctx[_core_id].channel == nullptr) {
        spdk_bdev_close(_desc);
        SPDK_ERRLOG("spdk_bdev_get_io_channe failed.\n");
        exit(1);
    }

    printf("polling_poller_register [poller_bdev_write][thread_id:%d][core_id:%d]!\n", _thread_id, _core_id);
    struct spdk_poller* _poller = spdk_poller_register(poller_bdev_write, (void*)&g_spdk_ctx[_core_id], 0);
    assert(_poller != nullptr);
    g_spdk_ctx[_core_id].q_poller.push(_poller);

    printf("polling_poller_register [poller_clean_cq][thread_id:%d][core_id:%d]!\n", _thread_id, _core_id);
    _poller = spdk_poller_register(poller_clean_cq, (void*)&g_spdk_ctx[_core_id], 0);
    assert(_poller != nullptr);
    g_spdk_ctx[_core_id].q_poller.push(_poller);
}

void start_app(void* cb)
{
    int _rc;
    struct spdk_bdev_opts __opts;
    spdk_bdev_get_opts(&__opts, sizeof(__opts));
    printf("bdev_pool_size:%d\n", __opts.bdev_io_pool_size);

    g_bdev = spdk_bdev_first();
    while (g_bdev != nullptr) {
        printf("module_name [%s]\n", spdk_bdev_get_module_name(g_bdev));
        g_bdev = spdk_bdev_next(g_bdev);
    }

    g_bdev = spdk_bdev_get_by_name(g_bdev_name);
    if (g_bdev == nullptr) {
        printf("spdk_bdev_get_by_name failed! [%s]\n", g_bdev_name);
        exit(1);
    } else {
        assert(g_bdev != nullptr);
        _rc = spdk_bdev_open_ext(g_bdev_name, true, bdev_event_cb, nullptr, &g_desc);
        printf("spdk_bdev_open [%s][bs:%zu][align:%zu]\n", g_bdev_name, spdk_bdev_get_block_size(g_bdev), spdk_bdev_get_buf_align(g_bdev));
        if (_rc) {
            printf("spdk_bdev_open_ext failed!\n");
            exit(1);
        } else {
            assert(g_desc != nullptr);
            printf("spdk_bdev_open_ext ok!\n");
        }
    }

    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("[START APPLICATION!][core_count:%d/%d][thread_count:%d/%d]\n",
        spdk_env_get_current_core(), spdk_env_get_core_count(), spdk_thread_get_id(spdk_get_thread()), spdk_thread_get_count());

    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, start_io_event, (void*)g_bdev, (void*)g_desc);
            spdk_event_call(event);
        }
    }
}

void stop_io_event(void* arg1, void* arg2)
{
    int _core_id = spdk_env_get_current_core();
    int _thread_id = spdk_thread_get_id(spdk_get_thread());

    printf("stop_event [thread_id:%d/core_id:%d][io_cnt:%d]\n",
        _thread_id, _core_id, g_spdk_ctx[_thread_id].io_cnt);

    while (!g_spdk_ctx[_core_id].q_poller.empty()) {
        struct spdk_poller* _poller = g_spdk_ctx[_core_id].q_poller.front();
        g_spdk_ctx[_core_id].q_poller.pop();
        spdk_poller_unregister(&_poller);
    }

    printf("free io channel.\n");
    assert(g_spdk_ctx[_core_id].channel != nullptr);
    spdk_put_io_channel(g_spdk_ctx[_core_id].channel);

    printf("exit thread!\n");
    struct spdk_thread* _thread = spdk_get_thread();
    spdk_thread_exit(_thread);
    g_num_run_thread--;
}

void stop_app()
{
    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("[STOP APPLICATION!][core_count:%d/%d][thread_count:%d/%d]\n",
        spdk_env_get_current_core(), spdk_env_get_core_count(), spdk_thread_get_id(spdk_get_thread()), spdk_thread_get_count());

    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, stop_io_event, nullptr, nullptr);
            spdk_event_call(event);
        }
    }

    while (g_num_run_thread) { }

    printf("bdev close!\n");
    spdk_bdev_close(g_desc);
    // printf("spdk_app_stop! (%d)\n", g_app_rc);
    // spdk_app_stop(g_app_rc);
    printf("spdk_app_fini");
    spdk_app_fini();
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
    g_app_rc = spdk_app_start(&_app_opts, start_app, nullptr);
    return 0;
}