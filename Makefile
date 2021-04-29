CC := g++

# PKG-CONFIG
SPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/build/lib
SPDK_PKG_CONFIG_PATH=$(SPDK_BUILD_DIR)/pkgconfig
SPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(SPDK_PKG_CONFIG_PATH)" pkg-config --cflags --libs spdk_bdev_nvme spdk_nvme spdk_bdev_malloc spdk_bdev_modules spdk_env_dpdk spdk_event spdk_bdev)
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
