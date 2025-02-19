# HPCC NS-3 simulator
This is an NS-3 simulator for [HPCC: High Precision Congestion Control (SIGCOMM' 2019)](https://rmiao.github.io/publications/hpcc-li.pdf). It also includes the implementation of DCQCN, TIMELY, DCTCP, PFC, ECN and Broadcom shared buffer switch.

We have update this simulator to support HPCC-PINT, which reduces the INT header overhead to just 1 byte. This improves the long flow completion time. See [PINT: Probabilistic In-band Network Telemetry (SIGCOMM' 2020)](https://liyuliang001.github.io/publications/pint.pdf).

It is based on NS-3 version 3.17.

## Quick Start

### Build
`./waf configure`

Please note if gcc version > 5, compilation will fail due to some ns3 code style.  If this what you encounter, please use:

`CC='gcc-5' CXX='g++-5' ./waf configure`

### Experiment config
Please see `mix/config.txt` for example. 

`mix/config_doc.txt` is a explanation of the example (texts in {..} are explanations).

`mix/fat.txt` is the topology used in HPCC paper's evaluation, and also in PINT paper's HPCC-PINT evaluation.

### Run
The direct command to run is:
`./waf --run 'scratch/third mix/dcqcn-10-19/config.txt'`

./waf --run 'scratch/third mix/dcqcn-10-19/config.txt





We provide a `run.py` for automatically *generating config* and *running experiment*. Please `python run.py -h` for usage.
Example usage:
`python run.py --cc hp --trace flow --bw 100 --topo topology --hpai 50`

python run.py --cc dcqcn --trace flow-1-10-29 --bw 100 --topo topology-1-10-29

To run HPCC-PINT, try:
`python run.py --cc hpccPint --trace flow --bw 100 --topo topology --hpai 50 --pint_log_base 1.05 --pint_prob 1`

## Files added/edited based on NS3
The major ones are listed here. There could be some files not listed here that are not important or not related to core logic.

`point-to-point/model/qbb-net-device.cc/h`: the net-device RDMA       部分-15% 改-50%（550行）   完成

`point-to-point/model/pause-header.cc/h`: the header of PFC packet    完成

`point-to-point/model/cn-header.cc/h`: the header of CNP                      完成

`point-to-point/model/pint.cc/h`: the PINT encoding/decoding algorithms       完成

`point-to-point/model/qbb-header.cc/h`: the header of ACK                     完成

`point-to-point/model/qbb-channel.cc/h`: the channel of qbb-net-device        完成

`point-to-point/model/qbb-remote-channel.cc/h`                                完成

`point-to-point/model/rdma-driver.cc/h`: layer of assigning qp and manage multiple NICs    完成

`point-to-point/model/rdma-queue-pair.cc/h`: queue pair                     待-内容少

`point-to-point/model/rdma-hw.cc/h`: the core logic of congestion control   待-1100行（800行）

`point-to-point/model/switch-node.cc/h`: the node class for switch     完成  

`point-to-point/model/switch-mmu.cc/h`: the mmu module of switch       完成

`network/utils/broadcom-egress-queue.cc/h`: the multi-queue implementation of a switch port    完成  （ 200行左右） 

`network/utils/custom-header.cc/h`: a customized header class for speeding up header parsing  完成

`network/utils/int-header.cc/h`: the header of INT    完成

`applications/model/rdma-client.cc/h`: the application of generating RDMA traffic   完成


 -    ns3问题  ?????? -->
 -  packet.cc   ---两种tag
 -  部分解决   makecallback 和 callback  tracedback
 - 



## Notes on other schemes
The DCQCN implementation is based on [Mellanox's implementation on CX4 and newer version](https://community.mellanox.com/s/article/dcqcn-parameters), which is slightly different from the DCQCN paper version.

The TIMELY implementation is based on our own understanding of the TIMELY paper. We believe we correctly implemented it. We use the parameters in the TIMELY paper. For parameters whose settings are absent in the TIMELY paper, we get from [this paper (footnote 4)](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/09/ecndelay-conext16.pdf).

The DCTCP implementation is a version that we envision DCTCP will be implemented in hardware. It starts at line rate (not slow start) which we believe is necessary in future high-speed network. It also does not delay ACK, because delayed ACk is for saving software overhead. These settings are consistent with other schemes.


python run.py --cc hp --trace flow --bw 100 --topo topology --hpai 50
python run.py --cc dcqcn --trace  flow --bw 100  --topo topology
-----------------------------------------------------------------------
ENABLE_QCN			Yes
USE_DYNAMIC_PFC_THRESHOLD	Yes
PACKET_PAYLOAD_SIZE		1000
TOPOLOGY_FILE			mix/topology.txt
FLOW_FILE			mix/flow.txt
TRACE_FILE			mix/trace.txt
TRACE_OUTPUT_FILE		mix/mix_topology_flow_hp95ai50.tr
FCT_OUTPUT_FILE		mix/fct_topology_flow_hp95ai50.txt
PFC_OUTPUT_FILE				mix/pfc_topology_flow_hp95ai50.txt
SIMULATOR_STOP_TIME		4
CC_MODE		3
ALPHA_RESUME_INTERVAL		1
RATE_DECREASE_INTERVAL		4
CLAMP_TARGET_RATE		No
RP_TIMER			300
EWMA_GAIN			0.00390625
FAST_RECOVERY_TIMES		1
RATE_AI				50Mb/s
RATE_HAI			50Mb/s
MIN_RATE		1000Mb/s
DCTCP_RATE_AI				1000Mb/s
ERROR_RATE_PER_LINK		0
L2_CHUNK_SIZE			4000
L2_ACK_INTERVAL			1
L2_BACK_TO_ZERO			No
HAS_WIN		1
GLOBAL_T		1
VAR_WIN		1
FAST_REACT		1
U_TARGET		0.95
MI_THRESH		0
INT_MULTI				4
MULTI_RATE				0
SAMPLE_FEEDBACK				0
PINT_LOG_BASE				1.01
PINT_PROB				1
RATE_BOUND		1
ACK_HIGH_PRIO		0
LINK_DOWN				0 0 0
ENABLE_TRACE				0
KMAX_MAP				 100000000000 1600 400000000000 6400
KMIN_MAP				 100000000000 400 400000000000 1600
PMAX_MAP				 100000000000 0.2 400000000000 0.2
BUFFER_SIZE				32
QLEN_MON_FILE				mix/qlen_topology_flow_hp95ai50.txt
QLEN_MON_START				2000000000
QLEN_MON_END				3000000000
maxRtt=4160 maxBdp=52000
Running Simulation.
