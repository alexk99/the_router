# PPPoE performance evaluation tests

A goal of this test is to evalute routing performance of PPPoE BRAS functionality of TheRouter.

## Test lab scheme
<img src="http://therouter.net/images/tests/pppoe_tests/perf_tests/pppoe_perf_lab2.png">

H4 machine creates PPPoE subscribers sessions using <a href="https://github.com/iKuaiNetworks/PPPoEPerf">
iKuaiNetworks/PPPoEPerf</a> and generates traffic using DPDK Pktgen. Pktgen is configured to send 64bytes
packets to random destination IP addresses from PPPoE subscriber IP address pool. Packets are send to therouter
uplink so TheRouter receives and encapsulate them using pppoe and finally send them back.

To evaluate maximum performance a number of tests were conducted to find the maximum number of packets
TheRouter can forward without packet loss. Packet loss was measured by comparing the number
of packets PktGen send (port 1) with the number of packets successfully received on the switch port
TheRouter sends PPPoE traffic to (port 2).

## Hardware configuration

### H4 (Pktgen, PPPoePerf) machine
 * Processor: Intel(R) Core(TM) i5-2400 CPU @ 3.10GHz
 * NIC: Intel X710-DA2
 * Ram: 8Gb DDR3
		
### H5 (TheRouter) machine
 * Processor: Intel(R) Core(TM) i7-5820K CPU @ 3.30GHz
 * NIC: Intel XL710-DA4
 * Ram: 32Gb 4x8

### AR1 switch
 * Arista 7050S-64

## Results

Number of PPPoE sessions - 30k

rate 80%, maximum pps 11868955
pkt send 1000000000 pkt received 999999690, lost 0,000031%
pkt send 1000000000 pkt received 999999671, lost 0,000032%

rate 90%, maximum pps 13359155
pkt send 1000000000 pkt received 999998702, lost 0,000129%
pkt send 1000000000 pkt received 999998727, lost 0,000127%

rate 92, maximum pps 13644242
pkt send 1000000000 pkt received 999998265, lost 0,000173%
pkt send 1000000000 pkt received 999998588, lost 0,000141%

rate 95, maximum pps 14127746
pkt send 1000000000 pkt received 979674258, lost 2,032574%
pkt send 1000000000 pkt received 979672488, lost 2,032751%

## Conclustion
TheRouter is able to forward 13.3Mpps of pppoe packets for 30k 
subsribers session on a desktop class processfor using only 6 cores.
