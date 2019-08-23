# Dynamic routing. Integration with Quagga or FRR.

Integration with Quagga/FRR is built on using Kernel network interfaces (KNI) on the DPDK side 
and <a href="http://www.nongnu.org/quagga/docs/docs-multi/zebra-FIB-push-interface.html">zebra FIB push interface</a> 
on the quagga's side.

<img src="http://therouter.net/images/quagga.png">

KNI interfaces are created for every router's virtual interfaces (VIF) that are going to participate in
the dynamic routing interaction with external routers. To do so 'kni' flag must be used in the creation
of a VIF. Kni flag instructs the router's core to create KNI interface in the linux kernel and forward to it
all packets that are destined to any ip address of the parent VIF. In other words all the conroll plane traffic
goes through the KNI interfaces, so Quagga BGP daemon can receive it and sends responses back to the world.

Once the quagga has received a route it will try to push it throuht its 'FIB push inteface'. TheRouter listens
on that interface for a route updates add install them into its main routing table. That updates
instruct router's data plane core to forward traffic to the right destinaion. So, control plane trafic
goes along slow path through KNI to the quagga and then back to the router through FPM interface. But data traffic
will always go along the fast path right through the router's core to a destination.

## Using a separate network namespace

Since linux host running TheRouter can be used for running other network related programs (for example dhcpd, radius, etc..)
that use different NICs and network different routes it would be very convinient to completly separate linux network stack and the TheRouter network
enviroment. Linux network namespace is ideal solution for that task. TheRouter and Quagga/FRR will be running in one
network space and linux and all standart programs it hosts will be running in the default network namespace. 
Therefore the dynamic routes that Quagga/FRR creates for TheRouter will be installed in the dedicated routing table 
and will not mess with linux routes.

First, a network namespace should be created:

	ip netns add tr

Second, TheRouter and Quagga/FRR components (bgpd, ospfd, zebra) should be started in the created namespace "tr" 
by using the following prexix for a startup command:

	ip netns exec tr <command>

For example, in order to run TheRouter in the network namespace named "tr" TheRouter should be started using the command:

	ip netns exec tr \<route_path>\the_router --proc-type=primary -c 0xF --lcores=...

Also, note that rcli and telnet command for confiring TheRouter and Quagga should be also executed in the "tr" namespace.
So, it's very convinient to define a bash variable and use it instead of typing 'ip netns exec tr' each time.

	export rvrf="ip netns exec tr"
	$rvrf rcli sh ip route
	$rvrf telnet localhost bgpd

## Quagga/FRR installation

Quagga or FRR must be complied with the --enable-fpm option.
For example, if you are installing Quagga using sources then just run:

	./configure --enable-fpm
	make
	make install

## Running Quagga/FRR

FRRs zebra component should be executed with -M fpm
command line option. (Note: Quaggas zebra doesn't need that option).

It's better to run Quagga/FRR and TheRouter in the same dedicated linux network
namespace. To run Quagga/FRR components in a namaspace just use a prefix
"ip netns exec tr" before executable file, where "tr" is the name of the namespace.

For example,

	ip netns exec tr /usr/local/sbin/bgpd

Once Quagga/FRR is started it should listen FPM port tcp/2620.

	ip netns exec tr netstat -an | grep 2620

# Configuration examples

## BGP

### Configure TheRouter

Note that "kni" flag is used.

	runtime {
		..
		vif add name p0 port 1 type untagged flags kni
		ip addr add 10.0.0.1/24 dev p0
		..
	}

Create a network namespace and set up the loopback interface:

	ip netns add tr
	export rvrf="ip netns exec tr"
	$rvrf ip link set up lo

Start TheRouter and setup kni interfaces.
Each kni interface should be set up and the proper MAC address 
should be configured after the router has started:

	router_run.sh /etc/router.conf
	$rvrf ip link set address 6C:B3:11:51:15:50 up rkni_p0
	
KNIs MAC address should be equal to the MAC address of the corresponding
VIF interface which can be found out using the "rcli sh vif"
command.

Check out the routing table. There are only directly connected routes and a default route there:

	h5 # $rvrf rcli sh ip route
	10.0.0.0/24 C dev p0 src 10.0.0.1
	10.0.1.0/24 C dev p1 src 10.0.1.1
	0.0.0.0/0 via 10.0.1.2 dev p1 src 10.0.1.1		

### Start Zebra on the router's host using the following config file

zebra.conf

	hostname h5
	password xxx
	! Turn off welcome messages
	no banner motd
	log file /var/log/quagga/zebra.log

### Start bgpd on the router's host using the following config file

bgpd.conf

	!
	hostname h5
	password xxx
	log syslog
	!
	router bgp 64512
	  bgp router-id 10.0.0.1
	  neighbor 10.0.0.3 remote-as 64513
	!
	line vty
	!

### Start external bgp router
	
Let's take a cisco router and configure bgp on it:

	router bgp 64513
	  no synchronization
	  bgp router-id 10.0.0.3
	  bgp log-neighbor-changes
	  network 10.12.0.0 mask 255.255.255.0
	  neighbor 10.0.0.1 remote-as 64512
	  no auto-summary

### Result

Let's check out that we received a bgp route:

	h5 / # $rvrf telnet localhost bgpd
	Trying 127.0.0.1...
	Connected to localhost.
	Escape character is '^]'.
	
	Hello, this is Quagga (version 1.0.20160315).
	Copyright 1996-2005 Kunihiro Ishiguro, et al.
	
	
	User Access Verification
	
	Password:
	h5> ena
	h5# sh ip bgp
	BGP table version is 0, local router ID is 10.0.0.1
	Status codes: s suppressed, d damped, h history, * valid, > best, = multipath,
	              i internal, r RIB-failure, S Stale, R Removed
	Origin codes: i - IGP, e - EGP, ? - incomplete
	
	   Network          Next Hop            Metric LocPrf Weight Path
	*> 10.12.0.0/24     10.0.0.3                 0              0 64513 i
	
	Displayed  1 out of 1 total prefixes

Then check out the main routing table:

	h5 # $rvrf rcli sh ip route
	10.0.0.0/24 C dev p0 src 10.0.0.1
	10.12.0.0/24 via 10.0.0.3 dev p0 src 10.0.0.1
	10.0.1.0/24 C dev p1 src 10.0.1.1
	0.0.0.0/0 via 10.0.1.2 dev p1 src 10.0.1.1	

! We've got a new prefix 10.12.0.0/24
	
## OSFP
Let's set up a test network consisting of three routers: h4, h5 and c2.
h4 and h5 are routers running TheRouter software, c2 is a cisco router.
On each router an ip address is assigned to a loopback interface: h4 has 4.4.4.4
address, h5 has 5.5.5.5 address and c2 has 2.2.2.2 address.
Then we can use this addresses to establish iBGP connectivity. For example
we will establish a iBGP session beetween C2 and H5 using ip addresses assigned to loopback interfaces
2.2.2.2 and 5.5.5.5. Also C2 will annouce network 10.33.33.0/24 via iBGP peer with H5 to illustrate
the using of loopback addresses.

	h5 src # $rvrf telnet localhost zebra
	h5>
	h5> sh ip route
	...
	O>* 2.2.2.2/32 [110/11] via 192.168.1.137, rkni_v3, 00:03:43
	...
	B>  10.33.33.0/24 [200/0] via 2.2.2.2 (recursive), 00:03:28
	  *                         via 192.168.1.137, rkni_v3, 00:03:28
	...

### Router h4 configuration

h4 OSPF 

	!
	! Zebra configuration saved from vty
	!   2017/12/28 23:21:54
	!
	hostname h4
	password xxx
	log file /var/log/quagga/ospfd.log
	no banner motd
	!
	!
	interface lo
	!
	interface rkni_v3
	!
	router ospf
	 ospf router-id 192.168.1.112
	 redistribute connected
	 redistribute static
	 network 4.4.4.4/32 area 0.0.0.0
	 network 192.168.1.0/24 area 0.0.0.0
	!
	line vty
	!

h4 BGP

	!
	! Zebra configuration saved from vty
	!   2017/12/29 17:21:26
	!
	hostname h4
	password xxx
	log syslog
	!
	router bgp 64512
	 bgp router-id 192.168.1.112
	 network 10.111.0.0/29
	 neighbor 2.2.2.2 remote-as 64512
	 neighbor 2.2.2.2 update-source 4.4.4.4
	 neighbor 5.5.5.5 remote-as 64512
	 neighbor 5.5.5.5 update-source 4.4.4.4
	 neighbor 192.168.1.3 remote-as 64513
	 exit
	!
	line vty
	!	

h4 TheRouter

	startup {
	  sysctl set mbuf 8192
	  sysctl set log_level 7
	
	  # LAG (slave ports 0,1)
	  port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 1,2
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 300
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	
	  sysctl set dhcp_relay_enabled 1
	}
	
	runtime {
	  # loopback
	  ip addr add 4.4.4.4/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # home network link (vlan3)
	  vif add name v3 port 0 type dot1q cvid 3 flags npf_on, kni
	  ip addr add 192.168.1.112/24 dev v3
	}

### Router h5 configuration

h5 OSPF

	!
	frr version 3.0
	frr defaults traditional
	!
	hostname h5
	password xxx
	log file /var/log/frr/ospfd.log
	no banner motd
	!
	!
	interface lo
	!
	interface rkni_v3
	!
	router ospf
	 ospf router-id 5.5.5.5
	 redistribute connected
	 redistribute static
	 network 5.5.5.5/32 area 0
	 network 192.168.1.0/24 area 0.0.0.0
	!
	line vty
	!

h5 BGP

	!
	frr version 3.0
	frr defaults traditional
	!
	hostname h5
	password xxx
	log syslog
	!
	!
	router bgp 64512
	 bgp router-id 192.168.1.111
	 neighbor 2.2.2.2 remote-as 64512
	 neighbor 2.2.2.2 update-source 5.5.5.5
	 neighbor 192.168.1.3 remote-as 64513
	!
	 vnc defaults
	  response-lifetime 3600
	  exit-vnc
	!
	!
	ip prefix-list pl_norm_nets seq 5 permit 0.0.0.0/0 le 24
	!
	route-map rm_in permit 10
	 match ip address prefix-list pl_norm_nets
	!
	route-map rm_in deny 20
	!
	line vty
	!

h5 TheRouter

	h5 src # cat /etc/router_ospf_loopback.conf
	startup {
	  # mbuf mempool size
	  sysctl set mbuf 8192
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  rx_queue port 1 queue 0 lcore 1
	  rx_queue port 1 queue 1 lcore 2
	  rx_queue port 1 queue 2 lcore 3
	
	  sysctl set log_level 8
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 300
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	}
	
	runtime {
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  vif add name v3 port 0 type dot1q cvid 3 flags kni
	  ip addr add 192.168.1.111/24 dev v3
	}

### Router c2 configuration

	interface Loopback0
	 ip address 2.2.2.2 255.255.255.0
	 ip ospf 1 area 0
	 
	...
	
	router ospf 1
	 log-adjacency-changes
	 redistribute connected
	 redistribute static
	 network 192.168.1.0 0.0.0.255 area 0.0.0.0
	!
	router bgp 64512
	 no synchronization
	 bgp log-neighbor-changes
	 network 10.33.33.0 mask 255.255.255.0
	 neighbor 4.4.4.4 remote-as 64512
	 neighbor 4.4.4.4 update-source Loopback0
	 neighbor 5.5.5.5 remote-as 64512
	 neighbor 5.5.5.5 update-source Loopback0
	 no auto-summary
	!
