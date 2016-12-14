# Work in progress.. Stay tuned.



# The router
TheRouter is a software packet router based on <a href="http://dpdk.org/">DPDK</a> an <a href="https://github.com/alexk99/npf">NPF libraries.</a>
Using modern software technique such as QSBR, lockless data structures and
<a href="https://github.com/efficient/libcuckoo">cuckoo hashing</a> enables the router to perform routing and NAT on hight packet rates up
to 6 Mpps using commodity hardware <a href="/source_nat.md">NAT perfomance benchmarks</a>

It supports:
 * ipv4 static and <a href="/quagga_bgp.md">dynamic routing</a>. Dynamic routing is based on integration with 
 <a href="http://www.nongnu.org/quagga">Quagga Routing Suite</a>
 * dot1q and qinq ethernet encapsulations
 * arp
 * icmp
 * pbr and multiple routing tables (work in progess)
 * statefull and stateless firewall and various forms of NAT. This functionality is based 
   on the improved version of <a href="https://github.com/rmind/npf">NetBSD packet filter</a>

# Installation
<a href="/install.md">Installation</a>

# Configuration
<a href="/conf_options.md">Configuration</a>

# Use cases
<a href="/use_cases.md">Use cases</a>