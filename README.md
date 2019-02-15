<p align="center">
  <img src="http://therouter.net/images/r.png" />
</p>

# Work in progress.. Stay tuned.

# TheRouter
TheRouter is a software packet router based on <a href="http://dpdk.org/">DPDK</a> an <a href="https://github.com/alexk99/npf">NPF libraries.</a>
Using modern software technique such as QSBR, lockless data structures and
<a href="https://github.com/efficient/libcuckoo">cuckoo hashing</a> enables 
the router to perform routing (<a href="https://github.com/alexk99/the_router/blob/master/routing_perf_tests.md">up tp 32Mpps</a>)
and NAT (<a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">up to 6 Mpps</a>) 
on high packet rates using commodity hardware.

## It supports

 * IPv4 static and <a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md">dynamic routing</a>. Dynamic routing is based on integration with <a href="http://www.nongnu.org/quagga">Quagga Routing Suite</a> or <a href="https://frrouting.org/">FRRouting</a>
 * Dot1q and qinq ethernet encapsulations
 * IP unnumbered 
 * ARP, proxy ARP
 * ICMP
 * PBR and multiple routing tables
 * Statefull and stateless firewall and various forms of NAT. This functionality is based 
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

## Broadband Remote Access Server (BRAS/BNG) features 

 * IPoE L2/L3 connected subscribers
 * PPPoE subscribers
 * IPoE vlan per subsriber with ip unnumbered support
 * Traffic shaping (Token bucket filter with extended burst value)
 * DHCP relay
 * Redirect subsribers traffic based on multiple routing tables and PBR
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
 * Redirect subsribers traffic based on multiple routing tables and PBR
 * Full RFC-compliant support PPPoE, LCP, IPCP, PAP, CHAP
 * Up to 40000 concurrent pppoe subscribers

- Configuration options
<a href="https://github.com/alexk99/the_router/blob/master/conf_options.md#pppoe-subscribers">PPPoE conf options</a>
- HOWTO
<a href="https://github.com/alexk99/the_router/blob/master/bras/pppoe_bras_howto_eng.md">PPPoE BRAS howto</a>
- Download
PPPoE enabled the_router binary for x86_64 (intel core family or higher)
It's on the very first builds, no production tests have been perfmored yet.
<a href="http://therouter.net/downloads/the_router.6cores.dpdk.17.11.1.pppoe.pppoe_a0.06.tar.gz">Download</a>

PPPoE roadmap:
 * production and performance tests
 * IPv6 support

## News

 * Another production case - <a href="https://github.com/alexk99/the_router/blob/master/border_bgp_service_provider_1.md">
 TheRouter as a BGP border router in a service provider network</a>

## VRRP

 * <a href="https://github.com/alexk99/the_router/blob/master/vrrp/vrrp_lab_1.md">Lab #1 results</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/vrrp/vrrp_lab_2.2.md">Lab #2 results</a>

## IPV6. IPv6 support is in a test phase.

 * <a href="https://github.com/alexk99/the_router/blob/master/ipv6/ipv6_lab1_address_assgning.md">Lab #1. IPV6 address assigning commands</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ipv6/ipv6_lab2_1_ND.md">Lab #2. Neighbor discovery protocol and the neigbor cache states</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ipv6/ipv6_lab6_static_and_connected_routes_simple_forwarding.md">Lab #3. Static and connected ipv6 routes</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ipv6/ipv6_bgp_lab.md">Lab #4. IPv6 BGP</a>

 - IPv6 rcli commands descriptions https://github.com/alexk99/the_router/blob/master/conf_options.md#ipv6
 - Test ipv6 build http://therouter.net/downloads/the_router.6cores.dpdk.17.11.1.dev.a0.67.tar.gz

## IPFIX flow accouting. Flow accounting support is in a test phase.

- Flow support configuration commands - https://github.com/alexk99/the_router/blob/master/conf_options.md#flow-accounting-ipfix
- Test build http://therouter.net/downloads/the_router.6cores.dpdk.17.11.1.dev.a0.67.tar.gz

## Performance tests

 * <a href="https://github.com/alexk99/the_router/blob/master/routing_perf_tests.md">Routing performance evaluation tests</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">NAT performance benchmarks</a>

# Install

 * <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo Install</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu 16.04 Install</a>

# Configuration
<a href="https://github.com/alexk99/the_router/blob/master/conf_options.md">Configuration</a>

# Use cases

 * <a href="https://github.com/alexk99/the_router/blob/master/source_nat.md">Source NAT</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md">Dymamic routing. Quagga and FRR</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/link_bonding.md">Link bonding</a>
 * BRAS <a href="https://github.com/alexk99/the_router/blob/master/bras/bras_howto_eng.md">howto</a>, 
 <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md">docs</a>

# Production cases

## Bizin
<a href="https://github.com/alexk99/the_router/blob/master/bizin_eng.md">Bizin. TheRouter as a core BGP router.</a>

Bizin. The first experience using TheRouter as a BGP core router in an internet service provider network.
Forwarding traffic between a BRAS cluster and a Google Cache Server.

## TheRouter as a BGP border router in a service provider network

<a href="https://github.com/alexk99/the_router/blob/master/border_bgp_service_provider_1.md">
 TheRouter as a BGP border router in a service provider network</a>
