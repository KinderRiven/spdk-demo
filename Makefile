CC := g++


# PKG-CONFIG
SPDK_BUILD_DIR=/home/hanshukai/import_libs/spdk/build/lib
PKG_CONFIG_PATH = $(SPDK_BUILD_DIR)/pkgconfig
DPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)") pkg-config --libs spdk_env_dpdk
SPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)") pkg-config --libs spdk_bdev

LINK_FLAGS := -lpthread -lrt -lnuma -ldl -luuid -lm -lisal

INC := -I./include

all: reactor_demo

reactor_demo: clean
	$(CC) --std=c++11 reactor_demo.cc -o reactor_demo -Wl,--no-as-needed $(INC) $(DPDK_LINK_FLAGS) $(SPDK_LINK_FLAGS) $(LINK_FLAGS) -Wl,--as-needed

clean:
	rm -rf reactor_demo

export:
    export LD_LIBRARY_PATH=./third-party/dpdk:./third-party/spdk
