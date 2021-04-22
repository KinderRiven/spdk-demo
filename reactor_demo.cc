/*
 * @Author: your name
 * @Date: 2020-09-17 15:32:04
 * @LastEditTime: 2021-04-22 16:58:41
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

int poller_function(void* num)
{
    int p = *((int*)num);
    printf("poller_function [%d]\n", p);
    return 0;
}

void start_event(void* arg1, void* arg2)
{
    int _rc;
    int _core_id = *((int*)arg1);
    printf("Fuck you, man! This is start event [core_%d].\n", _core_id);

    // char _name[128];
    // sprintf(_name, "FuckYouMan_%d", _core_id);
    // spdk_thread_create(_name, nullptr);
}

void start_app(void* cb)
{
    printf("START APPLICATION!\n");

    int _v;
    struct spdk_thread* _spdk_thread;
    _spdk_thread = spdk_get_thread();
    printf("spdk_thread [core_count:%d]\n", spdk_env_get_core_count());

    // 对不同的核发起请求
    app_argv_t* _argv = (app_argv_t*)cb;
    for (int i = 0; i < _argv->num_reactor; i++) { // master reactor可以在其他核上发起一个事件
        int* core = (int*)malloc(sizeof(int));
        *core = (i + 1);
        struct spdk_event* event = spdk_event_allocate(i, start_event, (void*)core, nullptr);
        spdk_event_call(event);
    }

    // poller
    int _num_poller = 3;
    struct spdk_poller* _poller[128];
    for (int i = 0; i < _num_poller; i++) {
        int* _argv = (int*)malloc(sizeof(int));
        *_argv = i;
        uint64_t _time = (i + 1) * 500000;
        printf("poller_register [core_id:%d][time:%llu]!\n", i, _time);
        _poller[i] = spdk_poller_register(poller_function, (void*)_argv, _time);
    }

#if 0
    for (int i = 0; i < _num_poller; i++) {
        spdk_poller_unregister(&_poller[i]);
    }
#endif
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
    printf("OPT [name:%s][file_name:%s][reactor_mask:%s][main_core:%d]\n",
        _app_opts.name, _app_opts.config_file, _app_opts.reactor_mask, _app_opts.main_core);

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

    printf("APP [num_reactor:%d]\n", _app_argv.num_reactor);
    _rc = spdk_app_start(&_app_opts, start_app, (void*)&_app_argv);
    printf("Reactor Exit! (%d)\n", _rc);
    spdk_app_stop(_rc);
    return 0;
}