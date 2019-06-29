# 1. Introduction

This howto describes the configuration of a BRAS server running TheRouter software
under Linux operating system.

The BRAS server is going to be integrated into a small test network and it 
should be performing the following tasks:

 * termination users QinQ ethernet vlans;
 * user authorization and authentication via RADIUS protocol;
 * redirection unauthorized  users traffic to a web server;
 * shaping users traffic;
 * forwarding traffic based on routes received via BGP protocol;
 * Announsing subscriber's /32 prefixes via BGP protocol;
 * SNAT - network address translation of users source private IP addresses into a pool of public/white
 IP addresses;
	
# 2. Configuring Linux OS and TheRouter software

Follow either <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo installation</a>
instructions or <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu installation</a>
instruction and install DPDK and TheRouter on a machine running Linux OS.

# 3. Test network scheme

The test network consists of Linux host H4, a border router connected to the internet and clients/subscribers.
H4 host runs TheRouter and other programs required for accomplishing our tasks: dhcpd, freeradius, mysql, quagga or frr.
Subscribers 1 and 2 are connected via dedicated vlans, subscribers 3 and 4 are connected via a shared L2 broadcast domain.

## 3.1. L2 network scheme

<img src="http://therouter.net/images/bras/bras howto l2.png">

## 3.2. L3 network scheme

<img src="http://therouter.net/images/bras/bras_howto_l3.png">

Two static subinterfaces (VIF) configured in a router's configuration file
are v3 and v20. V3 VIF connects TheRouter to the border router.
v20 VIF connects TheTouter to H4 Linux network stack which runs on 
the same machine as TheRouter does,  but uses its own NICs.

# 4. Starting TheRouter, checking L2 and L3 connectivity.

## 4.1. Starting TheRouter

	ip netns add tr
	export rvrf="ip netns exec tr"
	$rvrf ip link set up lo
	$rvrf ip route flush dev lo
	
	$rvrf the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' \
	  --syslog='daemon' -n2 -w 0000:60:00.0 -w 0000:60:00.1 \
	  --vdev 'eth_bond0,mode=4,slave=0000:60:00.0,slave=0000:60:00.1,xmit_policy=l23' \
	  -- -d -c /etc/router_bras_dhcp_relay_lag.conf

	sleep 5
	$rvrf ip link set up dev rkni_v3 address 00:1B:21:3C:69:44 


## 4.2. TheRouter's configuration file

A detailed description of configuration commands 
will be provided in the following paragraphs.

/etc/router_bras_dhcp_relay_lag.conf

	startup {
	  sysctl set mbuf 8192
	  sysctl set log_level 7
	
	  #
	  # port and queues setup
	  #
	  # LAG (slave ports 0,1)
	  port 2 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 0,1
	
	  rx_queue port 2 queue 0 lcore 1
	  rx_queue port 2 queue 1 lcore 2
	  rx_queue port 2 queue 2 lcore 3
	
	  #
	  # NPF timeouts
	  #
	
	  # any protocol timeouts (UDP)
	  sysctl set NPF_ANY_CONN_CLOSED 0
	  sysctl set NPF_ANY_CONN_NEW 30
	  sysctl set NPF_ANY_CONN_ESTABLISHED 60
	
	  # TCP timeouts
	  sysctl set NPF_TCPS_CLOSED 10
	  sysctl set NPF_TCPS_SYN_SENT 30
	  sysctl set NPF_TCPS_SIMSYN_SENT 30
	  sysctl set NPF_TCPS_SYN_RECEIVED 60
	  sysctl set NPF_TCPS_ESTABLISHED 14400
	  sysctl set NPF_TCPS_FIN_SENT 240
	  sysctl set NPF_TCPS_FIN_RECEIVED 240
	  sysctl set NPF_TCPS_CLOSE_WAIT 45
	  sysctl set NPF_TCPS_FIN_WAIT 60
	  sysctl set NPF_TCPS_CLOSING 30
	  sysctl set NPF_TCPS_LAST_ACK 30
	  sysctl set NPF_TCPS_TIME_WAIT 120
	
	  #
	  # IPFIX accounting
	  #
	  sysctl set flow_acct 1
	  sysctl set flow_acct_dropped_pkts 1
	  sysctl set flow_idle_timeout 20
	
	  #
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 300
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set dhcp_relay_enabled 1
	  sysctl set fpm_debug 1
	  
	  # add/remove linux kernel /32 routes for ppp subscribers ip addresses.
	  # Linux kernel routes are installed to 'lo' interface in the namespace therouter is running in.
	  # This option allows to announce subscriber's /32 prefixes by using "redisribute kernel" command
	  # in FRR/Quagga bgpd or ospfd daemons. 	  
	  sysctl set install_subsc_linux_routes 1
	}
	
	runtime {
	  #
	  # Flow accounting
	  #
	  flow ipfix_collector addr 192.168.1.165
	
	  #
	  # Interfaces
	  #
	
	  # loopback
	  ip addr add 4.4.4.4/32 dev lo
	
	  # blackhole nat address
	  ip route add 10.111.0.1/32 unreachable
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole ipoe subscriber's prefix
	  ip route add 10.10.0.0/24 unreachable
	
	  # link with local linux host
	  vif add name v20 port 2 type dot1q cvid 20
	  ip addr add 192.168.20.1/24 dev v20
	
	  # uplink
	  vif add name v3 port 2 type dot1q cvid 3 flags npf_on, kni, flow_acct
	  ip addr add 192.168.1.112/24 dev v3
	  #ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112
	
	  # L3 connected subscribers (cisco)
	  vif add name v21 port 2 type dot1q cvid 21 flags kni,l3_subs
	  ip addr add 192.168.21.1/24 dev v21
	
	  # L2 connected subsribers (zyxel 5Ghz WiFi)
	  vif add name v5 port 2 type dot1q cvid 5 flags kni,l2_subs,proxy_arp
	  ip addr add 192.168.5.1/32 dev v5
	  ip route add 192.168.5.1/32 local
	
	  #
	  # DHCP relay
	  #
	  dhcp_relay opt82 mode rewrite_if_doesnt_exist
	  dhcp_relay opt82 remote_id "tr_h4"
	  dhcp_relay 192.168.20.3
	
	  #
	  # Radius
	  #
	  radius_client add src ip 192.168.20.1
	  radius_client add server 192.168.20.3
	  radius_client set secret "secret"
	  coa server set secret "secret"
	
	  #
	  # Blocked subscriber's route table
	  #
	  ip route table add rt_bl
	  ip route add 192.168.1.0/24 dev v3 src 192.168.1.112 table rt_bl
	  ip route add 0.0.0.0/0 via 192.168.1.230 src 192.168.1.112 table rt_bl
	
	  #
	  # PBR
	  #
	  u32set create ips1 size 4096 bucket_size 16
	  u32set create l2s1 size 4096 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  # PBR rules
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl
	  ip pbr rule add prio 20 u32set l2s1 type "l2" table rt_bl
	
	  #
	  # NAT events
	  #
	  sysctl set ipfix_nat_events 1
	  ipfix_collector addr 192.168.20.2
	
	  #
	  # NPF (NAT)
	  #
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}


## 4.3. Connectivity 

### 4.3.1. Make sure that the interfaces described in the configuration file are up and running:

	h4 ~ # $rvrf rcli sh vif
	vif v3
	  id 3
	  port 2, vlan 0.3, encapsulation dot1q
	  mac address 00:1B:21:3C:69:44
	  NPF index 10
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags KNI,NPF,FLOW
	  mtu 1500
	vif v5
	  id 5
	  port 2, vlan 0.5, encapsulation dot1q
	  mac address 00:1B:21:3C:69:44
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags PA,KNI,L2P
	  mtu 1500
	vif lo
	  id 1
	  port 255, vlan 4095.4095, encapsulation qinq
	  mac address 00:00:00:00:00:01
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags none
	  mtu 1500
	vif v21
	  id 4
	  port 2, vlan 0.21, encapsulation dot1q
	  mac address 00:1B:21:3C:69:44
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags KNI,L3P
	  mtu 1500
	vif v20
	  id 2
	  port 2, vlan 0.20, encapsulation dot1q
	  mac address 00:1B:21:3C:69:44
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags none
	  mtu 1500

### 4.3.2. ARP

	h4 ~ # $rvrf rcli sh arp
	port    vid     ip      mac     type    state
	2       0.3     192.168.1.165   E0:D5:5E:8D:35:2E       dynamic ok
	2       0.5     192.168.5.101   A8:5B:78:09:0C:E1       dynamic ok
	2       0.3     192.168.1.3     D4:CA:6D:7C:D0:DF       dynamic ok
	2       0.20    192.168.20.3    E0:D5:5E:8D:35:2E       dynamic ok
	2       0.20    192.168.20.2    60:A4:4C:41:0A:24       dynamic ok

### 4.3.3. ICMP

	h4 ~ # $rvrf rcli ping -c3 192.168.1.3
	Ping 192.168.1.3 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.418 ms
	reply 56 bytes icmp_seq=2 time=0.204 ms
	reply 56 bytes icmp_seq=3 time=0.203 ms
	---
	Ping 192.168.1.3 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.203/0.275/0.418
	h4 ~ #
	h4 ~ # $rvrf rcli ping -c3 192.168.20.3
	Ping 192.168.20.3 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.208 ms
	reply 56 bytes icmp_seq=2 time=0.206 ms
	reply 56 bytes icmp_seq=3 time=0.205 ms
	---
	Ping 192.168.20.3 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.205/0.206/0.208

## 4.4. KNI interfaces and FRR/Quagga integration

The detailed description of the way TheRouter communicates with FRR/Quagga is described on the page 
<a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite">
Dynamic routing. Integration with FRR/Quagga routing suite
</a>

### 4.4.1. KNI interfaces

A KNI interface must be created for each router's VIF that is gonna
be used in communication with other routers via a dynamic routing protocol 
supported by FRR/Quagga. An example of such interface is the v3 interface. 
The router receives a default route from the BGP peer established 
via the v3 KNI interface.

You should manually up KNI interfaces and configure their 
MAC addresses once TheRouter has started. MAC address of a KNI interface 
should be equal to the MAC address of the router's interface coupled with the KNI.
To figure out the MAC address of a VIF from use the output of the 'rcli sh vif' command.

Example of a bash script that up kni interfaces and configure their MAC address:

	#!/bin/bash
	
	ip link set up dev rkni_v3 address 00:1B:21:3C:69:44
	ip link set up dev rkni_v5 address 00:1B:21:3C:69:44
	ip link set up dev rkni_v21 address 00:1B:21:3C:69:44

### 4.4.2. FRR/Quagga

Zebra and bpgd FRR/Quagga daemons should be configured and started so TheRouter can
communicate with other BGP routers. Since TheRouter has been started in "tr" Linux
network namespace those daemons must also be started in the same namespace.

Zebra's configuration file /etc/quagga/zebra.conf

	hostname h4
	
	! Set both of these passwords
	password xxx
	enable password xxx
		
	! Turn off welcome messages
	no banner motd
	log file /var/log/quagga/zebra.log

Using a separate network namespace is required so TheRouter's routes don't conflict 
with routes Linux stack uses for its own network operations. The routes 
FRR/Quagga receives via KNI interfaces and installs into the main
routing table of "tr" namespace are meant only for TheTouter software and 
have no value for the other application running on the same linux host.
Zebra will pass that routes to TheRouter via "zebra FIB push interface".

Run zebra

	/etc/init.d/zebra start

Once zebra has stated, run bgpd.

Bgpd's configuration file /etc/quagga/bgpd.conf

	h4# cat /etc/quagga/bgpd.conf
	!
	! Zebra configuration saved from vty
	!   2017/05/09 15:50:41
	!
	hostname h4
	password xxx
	log syslog
	!
	router bgp 64512
	 bgp router-id 192.168.1.112
	 network 10.111.0.0/29
	 neighbor 192.168.1.3 remote-as 64513
	 exit
	!
	line vty
	!

A single BGP peer with the uplink border router 192.168.1.3 
is described in this configuration file. 
TheRouter announces network 10.111.0.0/29 which is used for 
SNAT function and serves as public addresses for subscribers. 
Note that a private prefix 10.111.0.0/29 is used only due to the 
test nature of this howto and lack of global public IP address. 
In a real environment, global public IP addresses are usually 
used in cases like this.

Run bgpd

	/etc/init.d/bgpd start
	
Ensure that a default (0.0.0.0/0) route is successfully received and 
installed into both the Linux routing table and TheRouter's routing table.

Linux routing table

	h4 src # $rvrf ip route ls
	default via 192.168.1.3 dev rkni_v3  proto zebra  metric 20

The TheRouter's main routing table:

	h4 src # $rvrf rcli sh ip route
	224.0.0.0/4 is unreachable
	192.168.21.0/24 C dev v21 src 192.168.21.1
	192.168.5.0/24 C dev v5 src 192.168.5.1
	192.168.20.0/24 C dev v20 src 192.168.20.1
	192.168.1.0/24 C dev v3 src 192.168.1.112
	10.10.0.0/24 is unreachable
	0.0.0.0/0 via 192.168.1.3 dev v3 src 192.168.1.112

# 4.4.2.1 Announcing subscriber's /32 prefixes

	To announce subscriber's ip routes via BGP those
	routes should be accessible to FRR/Quagga software.
	To make the routes accessible TheRouter install them
	at Linux lo interfaces in the network namespace, it is running in.
	There is a sysctl variable named "install_subsc_linux_routes" that controls
	whether or not TheRouter will install those linux routes.
	
	Once subscriber's routes are installed on linux lo interfaces
	they could be redistributed via BGP protocol. To configure
	the FRR to redistribute such routes use "redistribute kernel" command.

# 5. RADIUS

## 5.1. TheRouter radius client configuration

Following commands define RADIUS server IP address,
source IP address of RADIUS router's requests,
shared radius secret and CoA secret

	radius_client add src ip 192.168.20.1
	radius_client add server 192.168.20.3
	radius_client set secret "secret"
	coa server set secret "secret"

The source IP address must be assigned to a TheRouter's interface which
connects TheRouter to a RADIUS server. It is the v20 interface since it 
connects TheRouter to the Linux host H4 
where the RADIUS server is running.

	# link with local linux host
	vif add name v20 port 0 type dot1q cvid 20
	ip addr add 192.168.20.1/24 dev v20

Ip address 192.168.20.3 is configured on the Linux side of vlan 20.

	9: vlan20@eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
	    link/ether e0:d5:5e:8d:35:2e brd ff:ff:ff:ff:ff:ff
	    inet 192.168.20.3/24 brd 192.168.20.255 scope global vlan20
	       valid_lft forever preferred_lft forever

## 5.2. Radius server configuration 

FreeRadius is used as a RADIUS server in this howto.
The FreeRadius project has very good documentation and there are a lot of configuration
examples available on the internet, so, excuse me for not providing any examples here.

But, I will provide the text of the main SQL query that illustrates using of TheRouter specific
radius attributes and I will provide the TheRouter's VAS dictionary.

/etc/raddb/sql/mysql/dialup.conf

	## router_bras_dhcp_relay.conf
	## pbr.
	authorize_reply_query = "SELECT 1, '%{SQL-User-Name}', 'therouter_ingress_cir', '200', '=' \
	UNION SELECT 2, '%{SQL-User-Name}', 'therouter_engress_cir', '200', '+=' \
	UNION SELECT 3, '%{SQL-User-Name}', 'therouter_ipv4_addr', GetIpoeUserService(%{request:therouter_port_id}, '%{request:therouter_outer_vid}', '%{request:therouter_inner_vid}'), '+=' \
	UNION SELECT 4, '%{SQL-User-Name}', 'therouter_ipv4_mask', '24', '+=' \
	UNION SELECT 5, '%{SQL-User-Name}', 'therouter_ip_unnumbered', '1', '+='\
	UNION SELECT 6, '%{SQL-User-Name}', 'therouter_install_subsc_route', 1, '+=' \

This SQL query is used by FreeRadius to query data required to form a radius response
to a TheRouter's RADIUS request to authorize a subscriber connected via a dedicated vlan
or to authenticate a L2/L3 subscriber. 
The information provided in this radius reply will be used by TheRouter 
to configure subscribers ip addresses, routes and PBR rules.

The following attributes are used only to configure a dynamic VIF (subscriber connected via
a dedicated vlan)

	therouter_ipv4_addr
	therouter_ipv4_mask
	therouter_ip_unnumbered
	therouter_ingress_cir
	therouter_engress_cir
	
MySql stored procedure GetIpoeUserService will calculate the subscriber's IP address
based on subscriber's VLAN id and the port number via it is connected to TheRouter.

The detailed description of the ip configuration of subscriber's connected via a dedicated VLAN is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#vlan-per-subscriber">Vlan per subscriber</a>

The following attributes are used only to configure a L2/L3 connected vlan

	therouter_install_subsc_route
	therouter_ingress_cir
	therouter_engress_cir

The description of L2/L3 subscribers IP configuration process is provided below in section 6.2.

### 5.2.2. FreeRadius dictionary

Add the following lines to the /etc/raddb/dictionary

	VENDOR       TheRouter     12345
	BEGIN-VENDOR TheRouter
	    ATTRIBUTE therouter_ingress_cir 1 integer
	    ATTRIBUTE therouter_engress_cir 2 integer
	    ATTRIBUTE therouter_ipv4_addr 3 integer
	    ATTRIBUTE therouter_ipv4_mask 4 integer
	    ATTRIBUTE therouter_outer_vid 5 integer
	    ATTRIBUTE therouter_inner_vid 6 integer
	    ATTRIBUTE therouter_ip_unnumbered 7 integer
	    ATTRIBUTE therouter_port_id 8 integer
	    ATTRIBUTE therouter_ipv4_gw 9 integer
	    ATTRIBUTE therouter_pbr 10 integer
	    ATTRIBUTE therouter_install_subsc_route 17 integer
	END-VENDOR   TheRouter

# 6. Configure subscriber's sessions or dynamic VIF

## 6.1. Dynamic "Vlan per subscriber" subscriber's interfaces

The detailed description of "vlan per subscriber" dynamic interfaces is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#vlan-per-subscriber">vlan-per-subscriber</a>.
I am going to briefly describe them according to the context of our test lab.

Dynamic interfaces (dynamic VIF or vlan per subscriber) are created automatically by TheRouter software in a response
to subscriber's packets received at a router port. They cannot be created via rcli interface or 
using a configuration file command. In order for the router to start creating dynamic VIFs it should
be configured on which port it should expect a subscriber's traffic by using flag "dynamic_vif".

	port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 1,2

When TheRouter receives on a such port a packet that doesn't match any known virtual interfaces (VIFs),
it creates a new dynamic VIF provided that creation of the VIF is authorized by the RADIUS
protocol. VLAN id values for the new VIF are gathered from the ethernet header of a packet triggered
the creation.

The RADIUS request to authorize a creation of a new dynamic VIF contains subscriber's VLAN data.
The RADIUS response should contain the data required to configure IP protocol of a new dynamic VIF.

### 6.1.1. Viewing subscribers 

	h4 ~ # $rvrf rcli sh sub
	vlan    ip      mode    port    ingress_car     egress_car      rx_pkts tx_pkts rx_bytes        tx_bytes
	0.130   10.10.0.19      1       2       200 mbit/s      200 mbit/s      0       0       0       0
	0.5     192.168.5.101   1       2       200 mbit/s      200 mbit/s      0       0       0       0
	0.5     192.168.5.103   1       2       200 mbit/s      200 mbit/s      0       0       0       0


### 6.1.2. Routing of Dynamic VIF

If a radius response to a dynamic VIF creation request includes the radius VSA attribute "therouter_ip_unnumbered",
TheRouter creates a route for a subscriber. The detailed description of this process is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#authorization-response">
Radius Authorization responce</a>.

Ensure that a route has been created:

	h4 ~ # $rvrf rcli sh ip route
	...
	10.10.0.19/32 C dev dvif2.0.130 src 10.10.0.1
	...

10.10.0.19 is the IP address value included in the radius reply.
dvif2.0.130 is the name of a dynamic interface. "2.0.130" substring of the name is 
a concatenation of values of the fields port_id, svid, cvid.

## 6.2. L2/L3 connected subscribers

### 6.2.1. L2/L3 subscribers
Detailed information about L2 subscriber sessions is located in 
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#l2-connected-subscribers">
L2 connected subscribers</a>, and L3 connected subscriber detailed descriptions is in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#l3-connected-subscribers">
L3 connected subscribers</a>

L2/L3 subscriber sessions belong to a VIF, therefore to create them a special flag should be added
to a VIF creation command. To allow the router to create L2 subscriber sessions the "l2_subs" flag
should be used. To allow the router to create L3 subscriber sessions the "l3_subs" flag
should be used.

In this test lab L2 subscriber sessions are created on VIF v5:

	vif add name v5 port 2 type dot1q cvid 5 flags kni,l2_subs,proxy_arp

L3 sessions are created on VIF v21:

	vif add name v21 port 2 type dot1q cvid 21 flags kni,l3_subs

L2/L3 subscriber's sessions are not a kind of VIF (virtual interface) 
therefore there is no need to assign an ip address to them.
L2/L3 subscriber's routing could be configured either by using parent VIF static connected routes or 
by using ip unnumbered scheme. In this howto ip unnumbered scheme is used.
Prefix 192.168.5.0/24 is reserved to assign ip addresses for L2/L3 subscribers connected via v5.
Address 192.168.5.1/32 is assigned to TheRouter v5 interface. Once a subscriber is connected
and authorized a /32 route is created.

	  ip addr add 192.168.5.1/32 dev v5
	  ip route add 192.168.5.1/32 local

TheRouter should be implicitly configured to create subscriber's ip routes /32 by using
the "therouter_install_subsc_route" radius attribute.

	h4 ~ # $rvrf rcli sh ip route
	192.168.5.103/32 C dev v5 src 192.168.5.1
	192.168.5.101/32 C dev v5 src 192.168.5.1
	...

The only difference between L2 and L3 subscriber sessions is that L2 sessions store
subscribers MAC address in addition to subscribers IP address. Storing a MAC address allows 
TheRouter (not implemented yet) to make sure that packets going through a session match 
the session MAC address, and to execute defined actions when a packet doesn't match a session.

L2/L3 sessions support the shaping of traffic going through them.

### 6.2.2. Viewing L2/L3 sessions

	h4 ~ # $rvrf rcli sh sub
	vlan    ip      mode    port    ingress_car     egress_car      rx_pkts tx_pkts rx_bytes        tx_bytes
	0.130   10.10.0.19      1       2       200 mbit/s      200 mbit/s      0       0       0       0
	0.5     192.168.5.101   1       2       200 mbit/s      200 mbit/s      0       0       0       0
	0.5     192.168.5.103   1       2       200 mbit/s      200 mbit/s      0       0       0       0

# 7. DHCP server and DHCP Relay configuration

TheRouter software supports DHCP Relay feature.

	dhcp_relay opt82 mode rewrite_if_doesnt_exist
	dhcp_relay opt82 remote_id "tr_h4"
	dhcp_relay 192.168.20.3

Ip address 192.168.20.3 is the address of H4 host where a DHCP server is running.
Additional information about dhcp_relay commands is <a href="https://github.com/alexk99/the_router/blob/master/conf_options.md#dhcp-relay">here</a>

## 7.1. DHCP server config /etc/dhcp/dhcpd.conf
	
This is an example of a DHCP server configuration file describing IP options for two subscribers.
DHCP server is listening for requests on V3 interface and identifies subscribers by their MAC addresses.


	#
	# Global parameters
	#
	default-lease-time 864000; # 10 days
	max-lease-time 864000;
	option domain-name "home";
	option domain-name-servers 192.168.1.3;
	ddns-update-style none;
	ddns-updates off;
	authoritative;
	
	shared-network dd {
	
		subnet 10.10.0.0 netmask 255.255.255.0 {
		    option subnet-mask 255.255.255.0;
		    option routers 10.10.0.1;
		
		    pool {
		        range 10.10.0.50 10.10.0.200;
		        default-lease-time 72000; # 20 hours
		        max-lease-time 72000;
		        allow unknown clients;
		    }
		}
		
		subnet 192.168.20.0 netmask 255.255.255.0 {
		}
	
	}
	
	host c1 {
	  hardware ethernet F8:32:E4:72:61:1B;
	  fixed-address 10.10.0.12;
	}
	
	host c2 {
	  hardware ethernet 00:88:65:36:39:4c;
	  fixed-address 10.10.0.11;
	}
	
## 7.2. DHCP response routing

In order to DHCP server responses successfully reach their destination
a DHCP server should know routes to networks DHCP requests are coming from.
In this example, DHCP requests are coming from network 10.10.0.0/24.
Therefore there should be a route to the destination 10.10.0.0/24 on Linux host H4.
The route should point to TheRouter that is connected via vlan 20, so the route 
should be added by the following command:

	ip route add 10.10.0.0/24 via 192.168.20.1

# 8. Redirecting unauthorized users traffic to a site

Traffic redirection is implemented via PBR mechanism (Policy-based routing) and described in
the <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#unauthorised-subscribers-traffic-control">Unauthorised subscribers traffic control</a>

# 9. NAT configuration

NAT configuration is conntrolled by a separate configuration file which is defined by the following command:

	npf load "/etc/npf.conf.bras_dhcp_relay"

File /etc/npf.conf.bras_dhcp_relay

	map v3 netmap 10.111.0.0/29
	
	#alg "icmp"
	
	group default {
	  pass stateful final on v3 all
	}
	
Command "map v3 netmap 10.111.0.0/29" defines NAT translation of source IP addresses of packets going through 
v3 interface. Translation replaces a source ip address with an address from the pool 10.111.0.0/29.
The new address of a packet is calculated by combining an original packet source address and the prefix 10.111.0.0/29.

	new address is: 10.111.0.0 plus first 8 bits from an original address.
	Where 8 is calculated as: 32 - 29 == 3 ^ 2 == 8, where 29 - is taken from the netmap command.

According to that rule, the same source ip address will always be translated to the same address from the defined address pool.
