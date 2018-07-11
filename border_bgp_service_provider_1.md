# TheRouter as a border router in a service provider network

TheRouter as a border router in a service provider network One of several border 
router running under linux was replaced by TheRouter since there were not enough 
resources to handle current traffic uning Linux kernel. TheRouter installed even 
on a very cheap hardware now forwards traffic without any problem.

TheRouter is connected to a core switch using LACP link aggregation port (4x10G) 
and forwards traffic between several VLANs according to the routing table 
containing a full table/view (700k) routes. BGP connections are handled by FRR 
routing software.

# TheRouter server

 - CPU: AMD FX(tm)-8350 Eight-Core Processor
 - RAM: 16G
 - NICs: two Intel X520-DA2

 
### Stat
Image is clickable
<img src="http://therouter.net/images/production/sp1/production_border_router.png">
