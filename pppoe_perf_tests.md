# PPPoE performance evaluation tests

A goal of this test is to evalute routing performance of PPPoE BRAS functionality of TheRouter.

## Test lab scheme
<img src="http://therouter.net/images/tests/pppoe_tests/perf_tests/pppoe_perf_lab2.png">

H4 machine creates PPPoE subscribers sessions using <a href="https://github.com/iKuaiNetworks/PPPoEPerf">
iKuaiNetworks/PPPoEPerf</a> and generates traffic using DPDK Pktgen. Pktgen is configured to send 64bytes
packets to random destination IP addresses from PPPoE subscriber IP address pool. Packets are send to therouter
uplink so TheRouter receives and encapsulate them using pppoe and finally send them to PPPoE subscribers.

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

| rate, % | pkt send | pkt received | maximum pps | lost, % |
| -- | -- | -- | -- | -- |
| 80 | 1000000000 | 999999690 | 11868955 | 0,000031 |
| 80 | 1000000000 | 999999671 | 11868955 | 0,000032 |
| 90 | 1000000000 | 999998702 | 13359155 | 0,000129 |
| 90 | 1000000000 | 999998727 | 13359155 | 0,000127 |
| 92 | 1000000000 | 999998265 | 13644242 | 0,000173 |
| 92 | 1000000000 | 999998588 | 13644242 | 0,000141 |
| 95 | 1000000000 | 979674258 | 14127746 | 2,032574 |
| 95 | 1000000000 | 979672488 | 14127746 | 2,032751 |

## Conclustion
TheRouter is able to forward 13.3Mpps of PPPoE packets for 30k 
subscribers sessions using a desktop class processor with 6 cores.
