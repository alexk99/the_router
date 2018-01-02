# Dynamic routing. Integration with Quagga or FRR.

Integration with Quagga is built on using Kernel network interfaces (KNI) on the DPDK side 
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
that use different NICs and network routes it would be very convinient to separate linux network stack and the TheRouter network
enviroment completly. Linux network namespace is ideal solution for that task. TheRouter and Quagga/FRR will be running an one
network space and linux and all programs it hosts will be running in the default network namespace. Therefore the dynamic routes 
that Quagga/FRR creates for TheRouter will be in the dedicate routing table and will not mess with linux routes.

First, a network namespace should be created.

	ip netns add tr

Second, run TheRouter and Quagga/FRR components (bgpd, ospfd, zebra) in the created namespace "tr" 
by using the following prexix for a startup command:

	ip netns exec tr <command>

For example, in order to run TheRouter in the "tr" network namespace it should be started using the command:

	ip netns exec tr \<route_path>\the_router --proc-type=primary -c 0xF --lcores=...

Also, note than rcli command should be also executed in the "tr" namespace.
So, it's very convinient to define a bash variable and use it instead of typing 'ip netns exec tr'
before rcli.

	export rvrf="ip netns exec tr"
	$rvrf ip link set up dev rkni_v3


## Quagga/FRR configuring

Quagga or FRR must be complied with the --enable-fpm option.
For example, if you are installing Quagga using sources then just run:

	./configure --enable-fpm
	make
	make install

# Configuration examples

## BGP

### Configuring TheRouter

 * router.conf

Note that "kni" flag is used.

	runtime {
		..
		vif add name p0 port 1 type untagged flags kni
		ip addr add 10.0.0.1/24 dev p0
		..
	}

 * start TheRouter

Each kni interface should be set up after the router has started.

	router_run.sh /etc/router.conf
	ip netns exec tr ip link set up lo
	ip netns exec tr ip link set up rkni_p0

or

	router_run.sh /etc/router.conf
	$rvrf ip link set up lo
	$rvrf ip link set up rkni_p0

 * Check a routing table. There are only directly connected routes and a default route.

	h5 # $rvrf rcli sh ip route
	10.0.0.0/24 C dev p0 src 10.0.0.1
	10.0.1.0/24 C dev p1 src 10.0.1.1
	0.0.0.0/0 via 10.0.1.2 dev p1 src 10.0.1.1		

### Start Zebra on the router's host using the following config file

 * zebra.conf

	hostname h5
	password xxx
	! Turn off welcome messages
	no banner motd
	log file /var/log/quagga/zebra.log

### Start bgpd on the router's host using the following config file

 * bgpd.conf

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

### Check TheRouter's routing table

 * Let's check that we received a bgp route:

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

 * Then check our main routing table:
 
		h5 # $rvrf rcli sh ip route
		10.0.0.0/24 C dev p0 src 10.0.0.1
		10.12.0.0/24 via 10.0.0.3 dev p0 src 10.0.0.1
		10.0.1.0/24 C dev p1 src 10.0.1.1
		0.0.0.0/0 via 10.0.1.2 dev p1 src 10.0.1.1	
		
	! We've got a new prefix 10.12.0.0/24
	
## OSFP

