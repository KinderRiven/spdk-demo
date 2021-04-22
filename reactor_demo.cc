/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-22 21:14:07
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

static std::queue<struct spdk_poller*> g_thread_poller_map[128];

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
    uint64_t _time = 800000UL;
    printf("poller_register [core_id:%d][time:%llu]!\n", _core_id, _time);
    struct spdk_poller* _poller = spdk_poller_register(poller_function, arg1, _time);
    assert(_poller != nullptr);
    g_thread_poller_map[spdk_thread_get_id(spdk_get_thread())].push(_poller);
}

void stop_event(void* arg1, void* arg2)
{
    int _thread_id = spdk_thread_get_id(spdk_get_thread());
    printf("Fuck you, man! stop_event [thread%d/core%d]\n", _thread_id, spdk_env_get_current_core());
    while (!g_thread_poller_map[_thread_id].empty()) {
        struct spdk_poller* _poller = g_thread_poller_map[_thread_id].front();
        g_thread_poller_map[_thread_id].pop();
        spdk_poller_unregister(&_poller);
    }
    spdk_thread_exit(spdk_get_thread());
}

void start_app(void* cb)
{
    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("[START APPLICATION!][core_count:%d/%d]\n", spdk_env_get_current_core(), spdk_env_get_core_count());

    SPDK_ENV_FOREACH_CORE(i)
    {
        if (spdk_env_get_current_core() != spdk_env_get_first_core()) {
            struct spdk_event* event = spdk_event_allocate(i, start_event, nullptr, nullptr);
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
        _app_opts.name, _app_opts.config_file, _app_opts.reactor_mask, _app_opts.main_core);
    _rc = spdk_app_start(&_app_opts, start_app, nullptr);
    spdk_app_fini();
    return 0;
}