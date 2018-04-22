# VRRP Lab 2.2. Two virtual routers on a the same vlan.

lab#2.1 test network consists of two VRRP routers: r2 - arista l3 switch 
and h5 - the_router. Routers participate in two virtual router group 10 and 11
running on the vlan 5. C5 and C6 are hosts that use a virtual router as
a default gateway. C5's default gateway is virtual router group 10 and C6's default
gateway is group 11. Groups 10 and 11 use two different ipv4 subnets 10.0.5.0/24 and 
10.0.55.0/24. Also there is a C4 host to test that VRRP works ok and C5 and C6 are
able to communicate with it.

Note: hosts c4 and c5 are not dedicated hosts but different network namespaces ($c4ns and c5ns) on
the same machine h5. Host c6 is a network namespace on the machine h4.

	export c4ns="ip netns exec c4"
	export c5ns="ip netns exec c5"

<img src="http://therouter.net/images/vrrp/lab_2_2.png">

## 1) Initial configuration.

	Arista's config:

	!
	interface Vlan5
	   ip address 10.0.5.2/24
	   ip address 10.0.55.2/24 secondary
	   vrrp 10 ip 10.0.5.10
	   vrrp 10 ip 10.0.5.11 secondary
	   vrrp 11 priority 150
	   vrrp 11 ip 10.0.55.10
	   vrrp 11 ip 10.0.55.11 secondary

	
TheRouter's config

	...
	
	runtime {
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	
	  vif add name v3 port 0 type dot1q cvid 3
	  ip addr add 192.168.1.111/24 dev v3
	
	  vif add name v4 port 0 type dot1q cvid 4
	  ip addr add 10.0.4.1/24 dev v4
	
	  vif add name v5 port 0 type dot1q cvid 5
	  ip addr add 10.0.5.1/24 dev v5
	  ip addr add 10.0.55.1/24 dev v5
	
	  # default route
	  ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.111
	
	  # vrrp group 10
	  vrrp create group 10 dev v5
	  vrrp group 10 dev v5 prio 150
	  vrrp group 10 dev v5 ip add 10.0.5.10
	  vrrp group 10 dev v5 ip add 10.0.5.11 secondary
	
	  # vrrp group 11
	  vrrp create group 11 dev v5
	  vrrp group 11 dev v5 prio 100
	  vrrp group 11 dev v5 ip add 10.0.55.10
	  vrrp group 11 dev v5 ip add 10.0.55.11 secondary
	}

Router's log:

	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: Loading configuration ...
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: Loading the runtime part of configuration file '/etc/router_vrrp_lab2.2.conf'...
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: add lba_addr 3232236031 for 3232235887/24
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: add lba_addr 167773439 for 167773185/24
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: add lba_addr 167773695 for 167773441/24
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: add lba_addr 167786495 for 167786241/24
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp added group 10 virtual mac address 00:00:5E:00:01:0A on port 0
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp group 10 is created on vif v5
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp group 10 on vif v5 changed state, init -> backup
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp added group 11 virtual mac address 00:00:5E:00:01:0B on port 0
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp group 11 is created on vif v5
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: lcore 0: vrrp group 11 on vif v5 changed state, init -> backup
	Apr 22 19:51:00 h5 the_router[32335]: ROUTER: router configuration file '/etc/router_vrrp_lab2.2.conf' successfully loaded
	Apr 22 19:51:03 h5 the_router[32335]: ROUTER: lcore 0: vrrp group 10 on vif v5 changed state, backup -> master

Arista's VRRP group state

	localhost#
	localhost#sh vrrp
	Vlan5 - Group 10
	  State is Backup
	  Virtual IP address is 10.0.5.10
	    Secondary Virtual IP address is 10.0.5.11
	  Virtual MAC address is 0000.5e00.010a
	  Advertisement interval is 1.000s
	  Preemption is enabled
	  Preemption delay is 0.000s
	  Preemption reload delay is 0.000s
	  Priority is 100
	  Master Router is 10.0.55.1, priority is 150
	  Master Advertisement interval is 1.000s
	  Master Down interval is 3.609s
	
	Vlan5 - Group 11
	  State is Master
	  Virtual IP address is 10.0.55.10
	    Secondary Virtual IP address is 10.0.55.11
	  Virtual MAC address is 0000.5e00.010b
	  Advertisement interval is 1.000s
	  Preemption is enabled
	  Preemption delay is 0.000s
	  Preemption reload delay is 0.000s
	  Priority is 150
	  Master Router is 10.0.5.2 (local), priority is 150
	  Master Advertisement interval is 1.000s
	  Master Down interval is 3.414s

TheRouter VRRP group state

	h5 ~ # rcli sh vrrp
	vif v5 - group 11
	  vif v5 - port 0, vid 0.5, type 0
	  state is backup
	  virtual mac address is 00:00:5E:00:01:0B
	  primary ip address is 10.0.55.10
	    secondary ip address is 10.0.55.11
	  advertisement interval is 1 sec
	  preemption is on
	  priority is 100
	  master router is 10.0.5.2, priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.609 sec
	
	vif v5 - group 10
	  vif v5 - port 0, vid 0.5, type 0
	  state is master
	  virtual mac address is 00:00:5E:00:01:0A
	  primary ip address is 10.0.5.10
	    secondary ip address is 10.0.5.11
	  advertisement interval is 1 sec
	  preemption is on
	  priority is 150
	  master router is 10.0.55.1 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

## 2) Checking connectivity

Make sure that initiall configuratio is ok and hosts c5 and c6 are able to ping
their default gataways (virtual router group 10 and 11) and to ping host c4.

### c5

90:e2:ba:00:36:ec - the_router mac address

	h5 ~ # $c5ns ip route ls
	default via 10.0.5.10 dev vlan5
	10.0.5.0/24 dev vlan5 proto kernel scope link src 10.0.5.3
	10.0.55.0/24 dev vlan5 proto kernel scope link src 10.0.55.3
	
	h5 ~ # $c5ns ping -f -c5 10.0.5.10
	PING 10.0.5.10 (10.0.5.10) 56(84) bytes of data.
	
	--- 10.0.5.10 ping statistics ---
	5 packets transmitted, 5 received, 0% packet loss, time 0ms
	rtt min/avg/max/mdev = 0.099/0.133/0.180/0.029 ms, ipg/ewma 0.176/0.119 ms

	h5 ~ # $c5ns arp -n
	Address                  HWtype  HWaddress           Flags Mask            Iface
	10.0.55.1                ether   90:e2:ba:00:36:ec   C                     vlan5
	10.0.5.10                ether   00:00:5e:00:01:0a   C                     vlan5
	10.0.55.11               ether   00:00:5e:00:01:0b   C                     vlan5
	10.0.5.1                 ether   90:e2:ba:00:36:ec   C                     vlan5
	10.0.55.10               ether   00:00:5e:00:01:0b   C                     vlan5
	10.0.5.11                ether   00:00:5e:00:01:0a   C                     vlan5
	
	h5 ~ # $c5ns ping -f -c5 10.0.4.3
	PING 10.0.4.3 (10.0.4.3) 56(84) bytes of data.
	
	--- 10.0.4.3 ping statistics ---
	5 packets transmitted, 5 received, 0% packet loss, time 0ms
	rtt min/avg/max/mdev = 0.172/0.187/0.196/0.008 ms, ipg/ewma 0.220/0.179 ms

### c6

00:1c:73:4d:de:45 - arista mac address

	h4 ~ # $c5ns ip route ls
	default via 10.0.55.10 dev vlan5
	10.0.55.0/24 dev vlan5  proto kernel  scope link  src 10.0.55.5
	h4 ~ #
	h4 ~ # $c5ns ping -f -c5 10.0.55.10
	PING 10.0.55.10 (10.0.55.10) 56(84) bytes of data.
	
	--- 10.0.55.10 ping statistics ---
	5 packets transmitted, 5 received, 0% packet loss, time 3ms
	rtt min/avg/max/mdev = 0.106/0.633/2.714/1.040 ms, ipg/ewma 0.788/1.637 ms
	h4 ~ #
	h4 ~ # $c5ns arp -n
	Address                  HWtype  HWaddress           Flags Mask            Iface
	10.0.55.10               ether   00:00:5e:00:01:0b   C                     vlan5
	10.0.55.1                ether   90:e2:ba:00:36:ec   C                     vlan5
	h4 ~ #
	h4 ~ # $c5ns ping -f -c5 10.0.4.3
	PING 10.0.4.3 (10.0.4.3) 56(84) bytes of data.
	.
	--- 10.0.4.3 ping statistics ---
	5 packets transmitted, 4 received, 20% packet loss, time 12ms
	rtt min/avg/max/mdev = 0.163/0.169/0.186/0.018 ms, ipg/ewma 3.071/0.168 ms

## Failover

### Initial state. TheRouter is a master for group 10 and arista is a master for group 11.

Dump traffic on host c4 to make sure that traffic originated on C5 comes through the_router
and traffic originated on c6 comes throuht arista.

	h5 ~ # $c4ns tcpdump -e -n -i vlan4
	dropped privs to tcpdump
	tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
	listening on vlan4, link-type EN10MB (Ethernet), capture size 262144 bytes
	20:04:08.437650 90:e2:ba:00:36:ec > ff:ff:ff:ff:ff:ff, ethertype ARP (0x0806), length 56: Request who-has 10.0.4.3 (ff:ff:ff:ff:ff:ff) tell 10.0.4.1, length 42
	20:04:08.437656 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype ARP (0x0806), length 42: Reply 10.0.4.3 is-at 60:a4:4c:41:0a:24, length 28
	20:04:08.437665 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.5.3 > 10.0.4.3: ICMP echo request, id 32363, seq 32, length 64
	20:04:08.437670 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.5.3: ICMP echo reply, id 32363, seq 32, length 64
	20:04:09.278835 00:1c:73:4d:de:45 > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.55.5 > 10.0.4.3: ICMP echo request, id 12693, seq 18, length 64
	20:04:09.278841 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.55.5: ICMP echo reply, id 12693, seq 18, length 64
	20:04:09.461646 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.5.3 > 10.0.4.3: ICMP echo request, id 32363, seq 33, length 64
	20:04:09.461650 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.5.3: ICMP echo reply, id 32363, seq 33, length 64
	20:04:10.278822 00:1c:73:4d:de:45 > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.55.5 > 10.0.4.3: ICMP echo request, id 12693, seq 19, length 64
	20:04:10.278827 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.55.5: ICMP echo reply, id 12693, seq 19, length 64


ICMP requests from C5 (10.0.5.3) have source MAC address 90:e2:ba:00:36:ec that is used by h5 (the_router).
ICMP requests from C6 (10.0.55.5) have source MAC address 00:1c:73:4d:de:45 that is use by R2 (arista).

### Failover. 

Block a switch port to which R2 (arista) is connected.
Now h5 the_router should become a master for group 11 and both hosts c5 and c6 should have it as a default gateway.

	DGS-1510-28X/ME:admin#config ports 28 state disable
	Command: config ports 28 state disable
	
	Success.

Check the_router vrrp state.

	h5 ~ # rcli sh vrrp
	vif v5 - group 11
	  vif v5 - port 0, vid 0.5, type 0
	  state is master
	  virtual mac address is 00:00:5E:00:01:0B
	  primary ip address is 10.0.55.10
	    secondary ip address is 10.0.55.11
	  advertisement interval is 1 sec
	  preemption is on
	  priority is 100
	  master router is 10.0.55.1 (this system), priority is 100
	  master router advertisement interval is 1 sec
	  master down interval is 3.609 sec
	
	vif v5 - group 10
	  vif v5 - port 0, vid 0.5, type 0
	  state is master
	  virtual mac address is 00:00:5E:00:01:0A
	  primary ip address is 10.0.5.10
	    secondary ip address is 10.0.5.11
	  advertisement interval is 1 sec
	  preemption is on
	  priority is 150
	  master router is 10.0.55.1 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

Dump traffic on host c4 to make sure that all packets come through the_router

	h5 ~ # $c4ns tcpdump -e -n -i vlan4
	dropped privs to tcpdump
	tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
	listening on vlan4, link-type EN10MB (Ethernet), capture size 262144 bytes
	20:20:59.325411 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.55.5 > 10.0.4.3: ICMP echo request, id 12918, seq 17, length 64
	20:20:59.325423 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.55.5: ICMP echo reply, id 12918, seq 17, length 64
	20:21:00.149795 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.5.3 > 10.0.4.3: ICMP echo request, id 32363, seq 1020, length 64
	20:21:00.149801 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.5.3: ICMP echo reply, id 32363, seq 1020, length 64
	20:21:00.325397 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.55.5 > 10.0.4.3: ICMP echo request, id 12918, seq 18, length 64
	20:21:00.325402 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.55.5: ICMP echo reply, id 12918, seq 18, length 64
	20:21:01.173659 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.5.3 > 10.0.4.3: ICMP echo request, id 32363, seq 1021, length 64
	20:21:01.173665 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.5.3: ICMP echo reply, id 32363, seq 1021, length 64
	20:21:01.325392 90:e2:ba:00:36:ec > 60:a4:4c:41:0a:24, ethertype IPv4 (0x0800), length 98: 10.0.55.5 > 10.0.4.3: ICMP echo request, id 12918, seq 19, length 64
	20:21:01.325398 60:a4:4c:41:0a:24 > 90:e2:ba:00:36:ec, ethertype IPv4 (0x0800), length 98: 10.0.4.3 > 10.0.55.5: ICMP echo reply, id 12918, seq 19, length 64


Now all ICMP requests (from 10.0.5.3 and from 10.0.55.5) have source MAC address 90:e2:ba:00:36:ec that is used by h5 (the_router).

