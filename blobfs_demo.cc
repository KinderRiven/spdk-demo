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

#include "spdk/blob.h"
#include "spdk/blob_bdev.h"
#include "spdk/blobfs.h"
#include "spdk/blobfs_bdev.h"

int main(int argc, char** argv)
{
    return 0;
}