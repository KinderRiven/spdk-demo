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
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

char* g_bdev_name;

struct app_msg_t {
public:
    char* bdev_name;
};

void write_cb(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    printf("[write_callback:%d]\n", success);
}

void start_app(void* cb)
{
    int rc;
    uint32_t blk_size;
    uint32_t buf_align;
    struct app_msg_t* msg = (struct app_msg_t*)cb;
    struct spdk_bdev* bdev;
    struct spdk_bdev_desc* desc;
    struct spdk_io_channel* channel;

    printf(">>>>[start_thread(0x%llx)]\n", (uint64_t)cb);
    bdev = spdk_bdev_get_by_name(msg->bdev_name);
    printf("    [bdev:%s|%llx]\n", msg->bdev_name, (uint64_t)bdev);
    if (bdev == nullptr) {
        printf("bad bdev!\n");
        exit(0);
    }

    rc = spdk_bdev_open(bdev, true, nullptr, nullptr, &desc);
    if (rc) {
        printf("bad bdev open!\n");
        exit(0);
    }

    channel = spdk_bdev_get_io_channel(desc);
    if (channel == nullptr) {
        printf("bad channel!\n");
        exit(0);
    }

    blk_size = spdk_bdev_get_block_size(bdev);
    buf_align = spdk_bdev_get_buf_align(bdev);
    void* buff = spdk_dma_zmalloc(blk_size, buf_align, nullptr);
    printf("[blk_size:%d][buf_align:%d][buff:%llx]\n", blk_size, buf_align, (uint64_t)buff);

    rc = spdk_bdev_write(desc, channel, buff, 0, blk_size, write_cb, nullptr);
    printf("%d\n", rc);
}

int bdev_parse_arg(int ch, char* arg)
{
    printf(">>>>[bdev_parse_arg(%c)-(%s)]\n", ch, arg);
    g_bdev_name = arg;
    return 0;
}

void bdev_usage()
{
    printf(">>>>[bdev_usage]\n");
}

int main(int argc, char** argv)
{
    int rc;
    struct app_msg_t app_msg;
    struct spdk_app_opts opts = {};

    spdk_app_opts_init(&opts);
    opts.name = "bdev-example";

    if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, bdev_parse_arg, bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(rc);
    }

    app_msg.bdev_name = g_bdev_name;
    printf("OPT [name:%s][file_name:%s]\n", opts.name, opts.config_file);
    printf("APP [name:%s]\n", app_msg.bdev_name);
    rc = spdk_app_start(&opts, start_app, (void*)&app_msg);
    spdk_app_stop(rc);
    return 0;
}