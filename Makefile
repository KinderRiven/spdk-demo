CC := g++


DPDK_LINK_FLAGS := -Wl,--whole-archive -L./third-party/dpdk \
    -lrte_eal -lrte_mempool -lrte_ring -lrte_mbuf -lrte_mempool_ring -lrte_pci \
    -lrte_bus_pci -lrte_kvargs -lrte_vhost -lrte_net -lrte_hash -lrte_telemetry \
    -lrte_cryptodev -lrte_power -lrte_rcu

SPDK_LINK_FLAGS := -Wl,--whole-archive -L./third-party/spdk \
    -lspdk_env_dpdk  -lspdk_env_dpdk_rpc -lspdk_json \
    -lspdk_jsonrpc -lspdk_rpc \
    -lspdk_event_net -lspdk_event_vmd -lspdk_event \
    -lspdk_thread -lspdk_sock_posix -lspdk_sock -lspdk_notify -lspdk_net -lspdk_nvme \
    -lspdk_log -lspdk_trace -lspdk_util -lspdk_conf -lspdk_vmd

# PKG-CONFIG
# SPDK_BUILD_DIR=../third-party/spdk 
# PKG_CONFIG_PATH = $(SPDK_BUILD_DIR)/pkgconfig
# DPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)") pkg-config --libs spdk_env_dpdk
# SPDK_LINK_FLAGS := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)") pkg-config --libs spdk_nvme

LINK_FLAGS := -lpthread -lrt -lnuma -ldl -luuid -lm -lisal -ltbb

INC := -I./include

all: reactor_demo

reactor_demo: clean
	$(CC) --std=c++11 reactor_demo.cc.cc -o reactor_demo.cc $(INC) $(DPDK_LINK_FLAGS) $(SPDK_LINK_FLAGS) $(LINK_FLAGS)

clean:
	rm -rf reactor_demo

export:
    export LD_LIBRARY_PATH=./third-party/dpdk:./third-party/spdk
