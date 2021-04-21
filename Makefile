CC := g++


# PKG-CONFIG
SPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/build/lib
SPDK_PKG_CONFIG_PATH = $(SPDK_BUILD_DIR)/pkgconfig
SPDK_LINK_FLAGS := -Wl,--whole-archive $(shell PKG_CONFIG_PATH="$(SPDK_PKG_CONFIG_PATH)" pkg-config --libs spdk_env_dpdk spdk_event spdk_bdev)

DPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/dpdk/build/lib
DPDK_PKG_CONFIG_PATH = $(DPDK_BUILD_DIR)/pkgconfig
DPDK_LINK_FLAGS := -Wl,--whole-archive $(shell PKG_CONFIG_PATH="$(DPDK_PKG_CONFIG_PATH)" pkg-config --libs libdpdk)

LINK_FLAGS := -lpthread -lrt -lnuma -ldl -luuid -lm -lisal

all: reactor_demo

reactor_demo: clean
	$(CC) --std=c++11 reactor_demo.cc $(LINK_FLAGS) -o reactor_demo -Wl,--no-as-needed $(DPDK_LINK_FLAGS) $(SPDK_LINK_FLAGS) -Wl,--as-needed

clean:
	rm -rf reactor_demo

export:
    export LD_LIBRARY_PATH=/home/hanshukai/import_libs/spdk/build/lib:/home/hanshukai/import_libs/spdk/dpdk/build/lib
