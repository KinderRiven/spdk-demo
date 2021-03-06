###
 # @Author: your name
 # @Date: 2021-04-29 19:28:23
 # @LastEditTime: 2021-04-30 13:02:16
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /spdk-demo/run_perf.sh
### 
# see : https://spdk.io/doc/nvme.html

PERF=/home/hanshukai/import_libs/spdk/build/examples/perf

# io_depth=1 for latency
# $PERF -q 1 -o 512 -w randread   -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 1024 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 2048 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 4096 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# # $PERF -q 1 -o 8192 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf
# $PERF -q 1 -o 65536 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q1.perf

$PERF -q 4 -o 512 -w randread   -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 1024 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 2048 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 4096 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 8192 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf
$PERF -q 4 -o 65536 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q4.perf

# io_depth=32 for bw
# PERF -q 8 -o 512 -w randread   -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 1024 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 2048 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 4096 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 8192 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf
# $PERF -q 8 -o 65536 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q8.perf

# io_depth=16 for bw
# $PERF -q 16 -o 512 -w randread   -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 1024 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 2048 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 4096 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 8192 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf
# $PERF -q 16 -o 65536 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q16.perf

# io_depth=32 for bw
# $PERF -q 32 -o 512 -w randread   -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 1024 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 2048 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 4096 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# PERF -q 32 -o 8192 -w randread  -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf
# $PERF -q 32 -o 65536 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 30 >> p4800_q32.perf