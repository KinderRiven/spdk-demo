/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-22 20:30:15
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

struct core_argv_t {
public:
    std::queue<struct spdk_poller*> vec_poller;
};

static core_argv_t g_core_argv[128];

int poller_function(void* argv)
{
    printf("poller_function [thread%d/core%d]\n", spdk_thread_get_id(spdk_get_thread()), spdk_env_get_current_core());
    return 0;
}

void start_event(void* arg1, void* arg2)
{
    int _core_id = spdk_env_get_current_core();
    printf("Fuck you, man! start_event [core_%d].\n", _core_id);

    // create thread
    char _name[128];
    sprintf(_name, "%d", _core_id);
    struct spdk_thread* _thread = spdk_thread_create(_name, NULL);
    spdk_set_thread(_thread);

    // poller register
    uint64_t _time = 500000UL;
    printf("poller_register [core_id:%d][time:%llu]!\n", _core_id, _time);
    struct spdk_poller* _poller = spdk_poller_register(poller_function, arg1, _time);
    g_core_argv[_core_id].vec_poller.push(_poller);
    assert(_poller != nullptr);
}

void stop_event(void* arg1, void* arg2)
{
    printf("Fuck you, man! stop_event [thread%d/core%d]\n", spdk_thread_get_id(spdk_get_thread()), spdk_env_get_current_core());

    int _core_id = spdk_env_get_current_core();
    while (!g_core_argv[_core_id].vec_poller.empty()) {
        struct spdk_poller* __poller = g_core_argv[_core_id].vec_poller.front();
        g_core_argv[_core_id].vec_poller.pop();
        spdk_poller_unregister(&__poller);
    }
    spdk_thread_exit(spdk_get_thread());
}

void start_app(void* cb)
{
    printf("START APPLICATION!\n");

    int i, v;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("spdk_thread [core_count:%d]\n", spdk_env_get_core_count());

    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, start_event, nullptr, nullptr);
            spdk_event_call(event);
        }
    }
}

void stop_app()
{
#if 0
    int i;
    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, stop_event, nullptr, nullptr);
            spdk_event_call(event);
        }
    }
#endif
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
        _app_opts.name, _app_opts.config_file, _app_opts.reactor_mask, _app_opts.main_core);
    _rc = spdk_app_start(&_app_opts, start_app, nullptr);
    spdk_app_fini();
    return 0;
}