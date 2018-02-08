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

 * ipv4 static and <a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md">dynamic routing</a>. Dynamic routing is based on integration with <a href="http://www.nongnu.org/quagga">Quagga Routing Suite</a> or <a href="https://frrouting.org/">FRRouting</a>
 * dot1q and qinq ethernet encapsulations
 * ip unnumbered 
 * arp, proxy arp
 * icmp
 * pbr and multiple routing tables
 * statefull and stateless firewall and various forms of NAT. This functionality is based 
   on the improved version of <a href="https://github.com/rmind/npf">NetBSD packet filter</a>
 * link bonding

## Broadband Remote Access Server (BRAS) features 

 * IPoE L2/L3 connected subscribers
 * IPoE vlan per subsriber with ip unnumbered adresses
 * traffic shaping (Token bucket filter with extended burst value)
 * DHCP relay
 * redirect subsribers traffic based on multiple routing tables and PBR
 * radius/coa
 * <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md">BRAS documentation</a>
 * <a href="https://github.com/alexk99/the_router/blob/master/bras/bras_howto_eng.md">BRAS howto</a>

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