/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-22 19:50:32
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /spdk-demo/reactor_demo.cc
 */
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
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

struct app_argv_t {
public:
    int num_reactor;
};

struct core_argv_t {
public:
    int id;
    struct spdk_poller* _poller;
    struct spdk_thread* _thread;
};

int poller_function(void* num)
{
    int p = *((int*)num);
    printf("poller_function [%d]\n", p);
    return 0;
}

void start_event(void* arg1, void* arg2)
{
    core_argv_t* _argv = (core_argv_t*)arg1;

    // id
    int _core_id = _argv->id;
    printf("Fuck you, man! This is start event [core_%d].\n", _core_id);

    // create thread
    char _name[128];
    sprintf(_name, "%d", _core_id);
    _argv->_thread = spdk_thread_create(_name, NULL);
    spdk_set_thread(_argv->_thread);

    // poller register
    uint64_t _time = 500000UL;
    printf("poller_register [core_id:%d][time:%llu]!\n", _core_id, _time);
    _argv->_poller = spdk_poller_register(poller_function, arg1, _time);
    assert(_argv->_poller != nullptr);
}

void start_app(void* cb)
{
    printf("START APPLICATION!\n");

    int i;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("spdk_thread [core_count:%d]\n", spdk_env_get_core_count());

    core_argv_t _core_argv[128];
    SPDK_ENV_FOREACH_CORE(i)
    {
        if (i != spdk_env_get_first_core()) {
            int* __core = (int*)malloc(sizeof(int));
            *__core = i;
            _core_argv[i].id = i;
            struct spdk_event* event = spdk_event_allocate(i, start_event, (void*)&_core_argv[i], nullptr);
            spdk_event_call(event);
        }
    }
}

int main(int argc, char** argv)
{
    int _rc;
    app_argv_t _app_argv;
    struct spdk_app_opts _app_opts = {};

    // 1.参数化参数
    spdk_app_opts_init(&_app_opts, sizeof(_app_opts));
    _app_opts.name = "FuckYouMan";

    // 2.参数解析
    if ((_rc = spdk_app_parse_args(argc, argv, &_app_opts, nullptr, nullptr, nullptr, nullptr)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(_rc);
    }

    // 3.Count
    {
        int _tmp, _num_reactor = 0;
        sscanf(_app_opts.reactor_mask, "%x", &_tmp);
        while (_tmp) {
            if (_tmp & 1) {
                _num_reactor++;
            }
            _tmp >>= 1;
        }
        _app_argv.num_reactor = _num_reactor;
    }

    printf("OPT [name:%s][file_name:%s][reactor_mask:%s][main_core:%d]\n",
        _app_opts.name, _app_opts.config_file, _app_opts.reactor_mask, _app_opts.main_core);
    printf("APP [num_reactor:%d]\n", _app_argv.num_reactor);
    _rc = spdk_app_start(&_app_opts, start_app, (void*)&_app_argv);
    spdk_app_fini();
    return 0;
}