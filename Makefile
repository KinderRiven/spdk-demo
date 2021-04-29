CC := g++

ALL_SPDK_LIBS := $("spdk_accel_ioat spdk_blobfs spdk_jsonrpc \
spdk_accel_modules     spdk_blob           spdk_log \
spdk_accel             spdk_conf           spdk_lvol \
spdk_bdev_aio          spdk_dpdklibs       spdk_nbd \
spdk_bdev_delay        spdk_env_dpdk       spdk_net \
spdk_bdev_error        spdk_env_dpdk_rpc   spdk_notify \
spdk_bdev_ftl          spdk_event_accel    spdk_nvme \
spdk_bdev_gpt          spdk_event_bdev     spdk_nvmf \
spdk_bdev_lvol         spdk_event_iscsi    spdk_rpc \
spdk_bdev_malloc       spdk_event_nbd      spdk_scsi \
spdk_bdev_modules      spdk_event_net      spdk_sock_modules \
spdk_bdev_null         spdk_event_nvmf     spdk_sock \
spdk_bdev_nvme         spdk_event          spdk_sock_posix \
spdk_bdev_passthru     spdk_event_scsi     spdk_syslibs \
spdk_bdev              spdk_event_sock     spdk_thread \
spdk_bdev_raid         spdk_event_vhost    spdk_trace \
spdk_bdev_split        spdk_event_vmd      spdk_util \
spdk_bdev_virtio       spdk_ftl            \
spdk_bdev_zone_block   spdk_ioat           spdk_vhost \
spdk_blob_bdev         spdk_iscsi          spdk_virtio \
spdk_blobfs_bdev       spdk_json           spdk_vmd")

# PKG-CONFIG
SPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/build/lib
SPDK_PKG_CONFIG_PATH=$(SPDK_BUILD_DIR)/pkgconfig
# SPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(SPDK_PKG_CONFIG_PATH)" pkg-config --cflags --libs spdk_bdev_nvme spdk_nvme spdk_bdev_malloc spdk_bdev_modules spdk_env_dpdk spdk_event spdk_bdev)
SPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(SPDK_PKG_CONFIG_PATH)" pkg-config --cflags --libs $(ALL_SPDK_LIBS))
SPDK_SYSLIB_FLAGS := $(shell PKG_CONFIG_PATH="$(SPDK_PKG_CONFIG_PATH)" pkg-config --cflags --libs --static spdk_syslibs)


DPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/dpdk/build/lib
DPDK_PKG_CONFIG_PATH=$(DPDK_BUILD_DIR)/pkgconfig
DPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(DPDK_PKG_CONFIG_PATH)" pkg-config --cflags --libs libdpdk)

LINK_FLAGS := -lpthread -lrt -lnuma -ldl -luuid -lm -lisal

all: reactor_demo bdev_demo

reactor_demo: clean
	$(CC) --std=c++11 reactor_demo.cc -o reactor_demo  $(LINK_FLAGS) -Wl,--whole-archive $(SPDK_LINK_FLAGS)  $(DPDK_LINK_FLAGS) -Wl,--no-whole-archive $(SPDK_SYSLIB_FLAGS)

bdev_demo: clean
	$(CC) --std=c++11 bdev_demo.cc -o bdev_demo  $(LINK_FLAGS) -Wl,--whole-archive $(SPDK_LINK_FLAGS)  $(DPDK_LINK_FLAGS) -Wl,--no-whole-archive $(SPDK_SYSLIB_FLAGS)

clean:
	rm -rf reactor_demo bdev_demo

export:
    export LD_LIBRARY_PATH=/home/hanshukai/import_libs/spdk/build/lib:/home/hanshukai/import_libs/spdk/dpdk/build/lib
