<p align="center">
  <img src="http://therouter.net/images/github_header5.png" />
</p>

# Work in progress.. Stay tuned.

TheRouter is a software packet router based on one of the fastest packet processing libraries. </a> Using modern software technique such as QSBR, lockless data structures and <a href="https://github.com/efficient/libcuckoo">cuckoo hashing</a> enables the router to perform routing (<a href="https://github.com/alexk99/the_router/blob/master/routing_perf_tests.md">up to 32Mpps</a>) and NAT (<a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">up to 6 Mpps</a>) on high packet rates using commodity hardware.

## It supports

 * IPv4 static and <a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md">dynamic routing</a>. Dynamic routing is based on integration with <a href="http://www.nongnu.org/quagga">Quagga Routing Suite</a> or <a href="https://frrouting.org/">FRRouting</a>
 * Dot1q and qinq ethernet encapsulations
 * IP unnumbered 
 * ARP, proxy ARP
 * ICMP
 * PBR and multiple routing tables
 * Stateful and stateless firewall and various forms of NAT. This functionality is based 
   on the improved version of <a href="https://github.com/rmind/npf">NetBSD packet filter</a>
 * Link Aggregation/Link Bonding/LACP
 * NAT events logging via IPFIX (https://tools.ietf.org/html/draft-ietf-behave-ipfix-nat-logging-13#page-11)
 * VRRP (v2 for IPv4 and v3 for IPv6)
 * IPv6 (test phase)
 	- Unicast routing
	- ICMPv6
	- Neighbor Discovery Protocol
	- 	Duplicate address detection
  	-	Neighbor Unreachability Detection
	- MLD v1, v2
	- SLAAC
	- VRRP v3
 * Flow accounting via IPFIX (test phase)

## BRAS/BNG - Broadband Remote Access Server  

 * IPoE L2/L3 connected subscribers
 * PPPoE subscribers
 * IPoE - VLAN per subscriber with IP unnumbered support
 * Traffic shaping (Token bucket filter with extended burst value)
 * DHCP relay
 * DHCP option82 rewrite function
 * Redirect subscribers traffic based on multiple routing tables and PBR
 * Radius/CoA
 * <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md">BRAS documentation</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/bras/bras_howto_eng.md">BRAS howto</a>

## PPPoE

PPPoE server supports:

 * RADIUS authorization (PAP and CHAP)
 * RADIUS accounting
 * CoA
 * Shaping (Token bucket filter with extended burst value)
 * TCP MSS FIX
 * Redirect subscribers traffic based on multiple routing tables and PBR
 * Full RFC-compliant support PPPoE, LCP, IPCP, PAP, CHAP
 * Up to 60000 concurrent PPPoE subscribers

Configuration

 * <a href="https://github.com/alexk99/the_router/blob/master/conf_options.md#pppoe-subscribers">PPPoE configuration options</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/bras/pppoe_bras_howto_eng.md">PPPoE BRAS howto</a>

Download

 * <a href="http://therouter.net/downloads/the_router.6cores.dpdk.17.11.1.pppoe.pppoe_a0.78.tar.gz">Download</a>
PPPoE enabled the_router binary for x86_64 (intel core family or higher)

PPPoE roadmap:

 * production tests
 * IPv6 support

## Performance tests

 * <a href="https://github.com/alexk99/the_router/blob/master/routing_perf_tests.md">Routing performance evaluation tests</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">NAT performance benchmarks</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/pppoe_perf_tests.md">PPPoE performance test</a>

## Install

 * <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo Install</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu 16.04 Install</a>

## Configuration

 * <a href="https://github.com/alexk99/the_router/blob/master/conf_options.md">Configuration</a>

## Use cases

 * <a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">Source NAT</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/border_bgp_service_provider_1.md">Border router</a>
 * IPoE BNG/BRAS <a href="https://github.com/alexk99/the_router/blob/master/bras/bras_howto_eng.md">howto</a>, 
 <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md">docs</a>
 * PPPoE BNG/BRAS <a href="https://github.com/alexk99/the_router/blob/master/bras/pppoe_bras_howto_eng.md">howto</a>

## Production cases

 * <a href="https://github.com/alexk99/the_router/blob/master/bizin_eng.md">Bizin. TheRouter as a core BGP router.</a>
The first experience of using TheRouter as a BGP core router in an internet service provider network.
Forwarding traffic between a BRAS cluster and a Google Cache Server.
 * <a href="https://github.com/alexk99/the_router/blob/master/border_bgp_service_provider_1.md">TheRouter as a BGP border router in a service provider network</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ash_pppoe_nat_isp.md">The first productive BRAS: PPPoE, NAT</a>
 
## Customers

| <a href="https://www.cloudbit.ch/"><img src="http://therouter.net/images/customers/cloudbit_250_130.png" /></a> | <a href="https://interra.ru/"><img src="http://therouter.net/images/customers/interra_250_130.png" /></a> | <a href="https://inetvl.ru"><img src="http://therouter.net/images/customers/atk_logo_250_130.png" /></a> |
| -- | -- | -- |

