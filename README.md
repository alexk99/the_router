# Work in progress.. Stay tuned.



# The router
The router is a software packet router based on DPDK an NPF libraries.
Using modern software technique such as QSBR, lockless data structures and
cuckoo hashing enables the router to perform routing and NAT on hight packet rates up
to 6 Mpps.

It supports:
 * ipv4 static and dynamic routing. Dynamic routing is based on integration with 
 <a href="http://www.nongnu.org/quagga">Quagga Routing Suite</a>
 * dot1q and qinq ehternet encapsulations
 * arp
 * icmp
 * pbr and multiple routing tables (work in progess)
 * statafull and stateless firewall and various form of NAT. This functionality is based 
   on improved version of <a href="https://github.com/rmind/npf">NetBSD packet filter</a>

# Installation
<a href="/install.md">Installation</a>

# Configuration
<a href="/conf_options.md">Configuration</a>

# Use cases
<a href="/use_cases.md">Use cases</a>