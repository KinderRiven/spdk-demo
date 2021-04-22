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

static char* g_bdev_name;
static int g_num_reactor;

struct app_msg_t {
public:
    char* bdev_name;
};

void write_cb(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    printf("[write_callback:%d]\n", success);
}

int tick_f1(void* num)
{
    uint64_t p = *((uint64_t*)num);
    printf("[tick_f1:%llu]\n", p);
    return 1;
}

int tick_f2(void* num)
{
    uint64_t p = *((uint64_t*)num);
    printf("[tick_f2:%llu]\n", p);
    return 1;
}

int tick_f3(void* num)
{
    uint64_t p = *((uint64_t*)num);
    printf("[tick_f3:%llu]\n", p);
    return 1;
}

void start_event(void* arg1, void* arg2)
{
    int* core = (int*)arg1;
    int c = *core;
    printf("Fuck you, man! This is core [%d].\n", c);
}

void start_app(void* cb)
{
    printf("start_app!\n");
    /*
    uint64_t* tick_1 = (uint64_t*)malloc(sizeof(uint64_t));
    *tick_1 = 500000;
    printf("poller_register (1)!\n");
    // time poller
    struct spdk_poller* poller_1 = spdk_poller_register(tick_f1, (void*)tick_1, *tick_1);

    uint64_t* tick_2 = (uint64_t*)malloc(sizeof(uint64_t));
    *tick_2 = 5000000;
    printf("poller_register (2)!\n");
    struct spdk_poller* poller_2 = spdk_poller_register(tick_f2, (void*)tick_2, *tick_2);

    uint64_t* tick_3 = (uint64_t*)malloc(sizeof(uint64_t));
    *tick_3 = 100000;
    printf("poller_register (3)!\n");
    struct spdk_poller* poller_3 = spdk_poller_register(tick_f3, (void*)tick_3, *tick_3);
    */

    for (int i = 0; i < g_num_reactor; i++) { // master reactor可以在其他核上发起一个事件
        int* core = (int*)malloc(sizeof(int));
        *core = i;
        struct spdk_event* event = spdk_event_allocate(i, start_event, (void*)core, nullptr);
        spdk_event_call(event);
    }
}

int bdev_parse_arg(int ch, char* arg)
{
    switch (ch) {
    case 'b':
        printf(">>>>[bdev_name(%c)-(%s)]\n", ch, arg);
        g_bdev_name = arg;
        break;
    default:
        break;
    }
    return 0;
}

void bdev_usage()
{
    printf(">>>>[bdev_usage]\n");
}

int main(int argc, char** argv)
{
    int _rc;
    struct app_msg_t _app_msg;
    struct spdk_app_opts _app_opts = {};

    // 1.参数化参数
    spdk_app_opts_init(&_app_opts, sizeof(_app_opts));
    _app_opts.name = "bdev-example";

    // 2.参数解析
    // #define SPDK_APP_GETOPT_STRING "c:de:ghi:m:n:p:r:s:uvA:B:L:RW:"
    /*
    static const struct option g_cmdline_options[] = {
    #define CONFIG_FILE_OPT_IDX	'c'
        {"config",			required_argument,	NULL, CONFIG_FILE_OPT_IDX},
    #define LIMIT_COREDUMP_OPT_IDX 'd'
        {"limit-coredump",		no_argument,		NULL, LIMIT_COREDUMP_OPT_IDX},
    #define TPOINT_GROUP_MASK_OPT_IDX 'e'
        {"tpoint-group-mask",		required_argument,	NULL, TPOINT_GROUP_MASK_OPT_IDX},
    #define SINGLE_FILE_SEGMENTS_OPT_IDX 'g'
        {"single-file-segments",	no_argument,		NULL, SINGLE_FILE_SEGMENTS_OPT_IDX},
    #define HELP_OPT_IDX		'h'
        {"help",			no_argument,		NULL, HELP_OPT_IDX},
    #define SHM_ID_OPT_IDX		'i'
        {"shm-id",			required_argument,	NULL, SHM_ID_OPT_IDX},
    #define CPUMASK_OPT_IDX		'm'
        {"cpumask",			required_argument,	NULL, CPUMASK_OPT_IDX},
    #define MEM_CHANNELS_OPT_IDX	'n'
        {"mem-channels",		required_argument,	NULL, MEM_CHANNELS_OPT_IDX},
    #define MASTER_CORE_OPT_IDX	'p'
        {"master-core",			required_argument,	NULL, MASTER_CORE_OPT_IDX},
    #define RPC_SOCKET_OPT_IDX	'r'
        {"rpc-socket",			required_argument,	NULL, RPC_SOCKET_OPT_IDX},
    #define MEM_SIZE_OPT_IDX	's'
        {"mem-size",			required_argument,	NULL, MEM_SIZE_OPT_IDX},
    #define NO_PCI_OPT_IDX		'u'
        {"no-pci",			no_argument,		NULL, NO_PCI_OPT_IDX},
    #define VERSION_OPT_IDX		'v'
        {"version",			no_argument,		NULL, VERSION_OPT_IDX},
    #define PCI_BLACKLIST_OPT_IDX	'B'
        {"pci-blacklist",		required_argument,	NULL, PCI_BLACKLIST_OPT_IDX},
    #define LOGFLAG_OPT_IDX	'L'
        {"logflag",			required_argument,	NULL, LOGFLAG_OPT_IDX},
    #define HUGE_UNLINK_OPT_IDX	'R'
        {"huge-unlink",			no_argument,		NULL, HUGE_UNLINK_OPT_IDX},
    #define PCI_WHITELIST_OPT_IDX	'W'
        {"pci-whitelist",		required_argument,	NULL, PCI_WHITELIST_OPT_IDX},
    #define SILENCE_NOTICELOG_OPT_IDX 257
        {"silence-noticelog",		no_argument,		NULL, SILENCE_NOTICELOG_OPT_IDX},
    #define WAIT_FOR_RPC_OPT_IDX	258
        {"wait-for-rpc",		no_argument,		NULL, WAIT_FOR_RPC_OPT_IDX},
    #define HUGE_DIR_OPT_IDX	259
        {"huge-dir",			required_argument,	NULL, HUGE_DIR_OPT_IDX},
    #define NUM_TRACE_ENTRIES_OPT_IDX	260
        {"num-trace-entries",		required_argument,	NULL, NUM_TRACE_ENTRIES_OPT_IDX},
    #define MAX_REACTOR_DELAY_OPT_IDX	261
        {"max-delay",			required_argument,	NULL, MAX_REACTOR_DELAY_OPT_IDX},
    #define JSON_CONFIG_OPT_IDX		262
        {"json",			required_argument,	NULL, JSON_CONFIG_OPT_IDX},
    };
    */
    if ((_rc = spdk_app_parse_args(argc, argv, &_app_opts, "b:", nullptr, bdev_parse_arg, bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(_rc);
    }

    // bdev name
    _app_msg.bdev_name = g_bdev_name;
    printf("OPT [name:%s][file_name:%s][reactor_mask:%s][main_core:%d]\n",
        _app_opts.name, _app_opts.config_file, _app_opts.reactor_mask, _app_opts.main_core);

    {
        int _tmp;
        sscanf(_app_opts.reactor_mask, "%x", &_tmp);
        g_num_reactor = 0;
        while (_tmp) {
            if (_tmp & 1) {
                g_num_reactor++;
            }
            _tmp >>= 1;
        }
    }

    printf("APP [name:%s][num_reactor:%d]\n", _app_msg.bdev_name, g_num_reactor);
    _rc = spdk_app_start(&_app_opts, start_app, (void*)&_app_msg);
    printf("Reactor Exit! (%d)\n", _rc);
    spdk_app_stop(_rc);
    return 0;
}