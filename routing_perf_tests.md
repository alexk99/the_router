# Routing performance evaluation tests

To evaluate maximum performance capability of TheRouter number of test was conducted.
Performed tests tried to cover dirrent types of ipv4 traffic and load that a router might face
in a real network. 

To generate test traffic PktGen software was used. PktGen is DPDK based stateless packet generator 
capable of creation traffic with different parameters
practically on wire speed. Main traffic parameters that was covered by tests are:
 - pps - packet per second
 - packet size
 - number of ip flows

Also different TheRouter configurations was tested that were differed by:
 - number of routes
 - port configuration

In order to put more pressure on TheRouter several different network schemes was tested:
 - unidirection traffic tests
 - bidirection traffic tests
 - 4-directional traffic tests
 - 4-directional LACP tests

### Unidirection and bidirectional test lab schemes
<img src="http://therouter.net/images/tests/routing_tests/27_01_2018/ub_tests.png">

### 4-direction test lab schemes
<img src="http://therouter.net/images/tests/routing_tests/27_01_2018/4dir_tests.png">

# Results
Test have showed that TheRouter is capable of forwarding up to 32Mpps traffic of 64 bytes packets with 0 packet loss 
(test 14.3 in the table with full results below). 

In more complicated test scenario when router was configured with LACP bonding port and 800k routes were installed into
its FIB it showed result of 23 Mpps with zero packet loss (test 17.1 and 17.7)

## Full results
 - <a href="http://therouter.net/images/tests/routing_tests/27_01_2018/router_test_21.01.2018_routing_test1_wo_borders.html">html table</a>

# Hardware configuration

## PktGen H3 machine
 * Processor: Intel(R) Core(TM) i5-2400 CPU @ 3.10GHz
 * NIC: Intel X520-DA2
 * Ram: 8Gb DDR3
	
## PktGen H5 machine
 * Processor: Intel(R) Core(TM) i5-3470 CPU @ 3.20GHz
 * NIC: Intel X520-DA2
 * Ram: 8Gb DDR3
	
## TheRouter R machine
 * Processor: Intel(R) Core(TM) i7-5820K CPU @ 3.30GHz
 * NIC: Intel XL710 40GbE
 * Ram: 32Gb 4x8

## AR1 switch
 * Arista 7050S-64
