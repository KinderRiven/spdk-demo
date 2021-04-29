<!--
 * @Author: your name
 * @Date: 2021-04-29 19:24:13
 * @LastEditTime: 2021-04-29 19:26:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /spdk-demo/perf.md
-->
./perf options [Kernel device(s)]...
	[-b allowed local PCIe device address]
	 Example: -b 0000:d8:00.0 -b 0000:d9:00.0
	[-q io depth]
	[-o io size in bytes]
	[-O io unit size in bytes (4-byte aligned) for SPDK driver. default: same as io size]
	[-P number of io queues per namespace. default: 1]
	[-U number of unused io queues per controller. default: 0]
	[-w io pattern type, must be one of
		(read, write, randread, randwrite, rw, randrw)]
	[-M rwmixread (100 for reads, 0 for writes)]
	[-L enable latency tracking via sw, default: disabled]
		-L for latency summary, -LL for detailed histogram
	[-l enable latency tracking via ssd (if supported), default: disabled]
	[-t time in seconds]
	[-a warmup time in seconds]
	[-c core mask for I/O submission/completion.]
		(default: 1)
	[-D disable submission queue in controller memory buffer, default: enabled]
	[-H enable header digest for TCP transport, default: disabled]
	[-I enable data digest for TCP transport, default: disabled]
	[-N no shutdown notification process for controllers, default: disabled]
	[-r Transport ID for local PCIe NVMe or NVMeoF]
	 Format: 'key:value [key:value] ...'
	 Keys:
	  trtype      Transport type (e.g. PCIe, RDMA)
	  adrfam      Address family (e.g. IPv4, IPv6)
	  traddr      Transport address (e.g. 0000:04:00.0 for PCIe or 192.168.100.8 for RDMA)
	  trsvcid     Transport service identifier (e.g. 4420)
	  subnqn      Subsystem NQN (default: nqn.2014-08.org.nvmexpress.discovery)
	  hostnqn     Host NQN
	 Example: -r 'trtype:PCIe traddr:0000:04:00.0' for PCIe or
	          -r 'trtype:RDMA adrfam:IPv4 traddr:192.168.100.8 trsvcid:4420' for NVMeoF
	[-e metadata configuration]
	 Keys:
	  PRACT      Protection Information Action bit (PRACT=1 or PRACT=0)
	  PRCHK      Control of Protection Information Checking (PRCHK=GUARD|REFTAG|APPTAG)
	 Example: -e 'PRACT=0,PRCHK=GUARD|REFTAG|APPTAG'
	          -e 'PRACT=1,PRCHK=GUARD'
	[-k keep alive timeout period in millisecond]
	[-s DPDK huge memory size in MB.]
	[-g use single file descriptor for DPDK memory segments]
	[-C max completions per poll]
		(default: 0 - unlimited)
	[-i shared memory group ID]
	[-Q log I/O errors every N times (default: 1)
	 -T, --logflag <flag>    enable log flag (all, json_util, log, nvme, opal, rpc, rpc_client, sock, vmd)
	[-V enable VMD enumeration]
	[-z disable zero copy send for the given sock implementation. Default for posix impl]
	[-Z enable zero copy send for the given sock implementation]
	[-A IO buffer alignment. Must be power of 2 and not less than cache line (64)]
	[-S set the default sock impl, e.g. "posix"]
	[-G enable debug logging (flag disabled, must reconfigure with --enable-debug)

perf -q 128 -o 4096 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30