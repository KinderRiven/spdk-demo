###
 # @Author: your name
 # @Date: 2021-04-29 19:28:23
 # @LastEditTime: 2021-04-29 19:48:16
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /spdk-demo/run_perf.sh
### 
# see : https://spdk.io/doc/nvme.html
# for latency
PERF=/home/hanshukai/import_libs/spdk/build/examples/perf
$PERF -q 1 -o 256 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 10
# $PERF -q 1 -o 4096 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 10
# $PERF -q 1 -o 16384 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 10
# $PERF -q 1 -o 32768 -w randread -r 'trtype:PCIe traddr:0000:86:00.0' -t 10