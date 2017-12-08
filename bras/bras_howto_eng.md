# 1. Introduction

This howto describes configuration of a BRAS server running TheRouter software
under Gentoo Linux operation system and using additional software: Dhcpd, FreeRadius, Mysql, Quagga.

The BRAS server is going to be integrated into a small test network and it should accomplish the following
goals:

 * termination users QinQ ethernet vlans;
 * user authorization and authentication via RADIUS protocol;
 * redirection unauthorized  users traffic to the a WEB server;
 * shaping users traffic;
 * forwarding traffic based on routes received via BGP protocol;
 * SNAT - network adress translation of users source private ip addresses into a pool of public/white
 ip addresses;
	
# 2. Configuration Gentoo Linux and TheRouter software

Following either <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo installation</a>
instructions or <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu installation</a>
instruction install DPDK and TheRouter on a machine running under Gentoo Linux or Ubuntu.

# 3. Test network scheme

The test network consists of a linux host H4, the border router connected to the internet and our clients/user/subscribers.
The H4 host runs TheRouter and other programs required for accomplishing our tasks: Dhcpd, FreeRadius, Mysql, Quagga.
Subscribers 1 and 2 are connected via dedicated vlans, subscribers 3 and 4 are connected via a shared L2 network/broadcast domain.

## 3.1. L2 network scheme

<img src="http://therouter.net/images/bras/bras howto l2.png">

## 3.2. L3 network scheme

<img src="http://therouter.net/images/bras/bras_howto_l3.png">

The two main static the_router's interfaces configured in a router's configuration file
are v3 and v20 virtual interfaces (VIF). VIF v3 connects the_router with the single uplink border router.
VIF v20 connects the_router with hosts H4 linux network stack which runs on the same host as the_router, but uses
his own NICs.

# 4. Starting TheRouter, checking L2 and L3 connectivity.

## 4.1. Starting TheRouter

the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' \
 --syslog='daemon' -n2 -w 0000:60:00.0 -w 0000:60:00.1  --vdev 'eth_bond0,mode=4,slave=0000:60:00.0,slave=0000:60:00.1,xmit_policy=l23' \
 -- -c /etc/router_bras_dhcp_relay_lag.conf

## 4.2. TheRouter's configuration file

Here is the router's configuration file.
Detailed descriptions of all configuration commands will be provided in the next paragraphs of this howto.

/etc/router_bras_dhcp_relay_lag.conf

	startup {
	  sysctl set mbuf 8192
	  sysctl set log_level 7
	
	  # LAG (slave ports 1,2)
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
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  radius_client add src ip 192.168.20.1
	
	
	  ip route add 10.10.0.0/24 unreachable
	
	  # link with local linux host
	  vif add name v20 port 0 type dot1q cvid 20
	  ip addr add 192.168.20.1/24 dev v20
	
	  # home network link (vlan3)
	  vif add name v3 port 0 type dot1q cvid 3 flags npf_on, kni
	  ip addr add 192.168.1.112/24 dev v3
	  #ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112
	
	  # cisco L3 connected
	  vif add name v21 port 0 type dot1q cvid 21 flags kni,l3_subs
	  ip addr add 192.168.21.1/24 dev v21
	
	  # L2 connected (zyxel 5Ghz WiFi)
	  vif add name v5 port 0 type dot1q cvid 5 flags kni,l2_subs
	  ip addr add 192.168.5.1/24 dev v5
	
	
	  ## static arp records
	  # radius server
	  arp add 192.168.20.2 90:e2:ba:4b:b3:17 dev v20 static
	
	  dhcp_relay 192.168.20.2
	
	  radius_client add server 192.168.20.2
	  radius_client set secret "secret"
	
	
	  # PBR
	  ip route table add rt_bl
	
	  u32set create ips1 size 4096 bucket_size 16
	  u32set create l2s1 size 4096 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl
	  ip pbr rule add prio 20 u32set l2s1 type "l2" table rt_bl
	
	
	  # NPF
	  # npf load "/etc/npf.conf.accept_all_2"
	
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}

## 4.3. Connnectivity checking 

### 4.3.1. Insure that all interfaces described in the configuration file are running:

	h4 src # rcli sh vif
	name    port    vid     mac                     type    flags   idx     ingress_car     egress_car
	v20     0       0.20    00:1B:21:3C:69:44       dot1q           10      -       -
	v5      0       0.5     00:1B:21:3C:69:44       dot1q   kni,l2 subs     13      -       -
	v3      0       0.3     00:1B:21:3C:69:44       dot1q   kni,NPF 11      -       -

### 4.3.2. ARP

	h4 src # rcli sh arp
	port    vid     ip      mac     type    state
	0       0.20    192.168.20.2    90:E2:BA:4B:B3:17       static  ok
	0       0.5     192.168.5.124   A8:5B:78:09:0C:E1       dynamic ok
	0       0.3     192.168.1.3     D4:CA:6D:7C:D0:DC       dynamic ok

### 4.3.3. ICMP

	h4 src # rcli ping -c3 192.168.1.3
	Ping 192.168.1.3 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.283 ms
	reply 56 bytes icmp_seq=2 time=0.279 ms
	reply 56 bytes icmp_seq=3 time=0.278 ms
	---
	Ping 192.168.1.3 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.278/0.280/0.283

	h4 src # rcli ping -c3 192.168.20.2
	Ping 192.168.20.2 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.501 ms
	reply 56 bytes icmp_seq=2 time=0.557 ms
	reply 56 bytes icmp_seq=3 time=0.279 ms
	---
	Ping 192.168.20.2 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.279/0.445/0.557

## 4.4. Starting KNI interfaces and Quagga integration

Detailed description of a way TheRouter communicates with Quagga provided on the page 
<a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite">
Dynamic routing. Integration with Quagga routing suite
</a>

### 4.4.1. KNI interfaces

KNI interfaces must be created for every router's VIF which will be used by the router to communicate
with other routers via any dynamic routing protocol supported by Quagga. In this howto example of such interface
is the v3 interface. The router receives a default route from the bgp peer established via the v3 kni interface.

You should up KNI interfaces and configure their MAC addresses after the_route has started.
MAC address on a kni interface should be equal to the mac address of the router's interface coupled with the kni
interface. MAC address of a VIF can be learned from 'rcli sh vif' command output.

Example of a bash script that up kni interfaces and configure their MAC address:

	#!/bin/bash
	ip link set up rkni_v3
	ip link set dev rkni_v3 address 00:1B:21:3C:69:44
	
	ip link set up rkni_v5
	ip link set dev rkni_v5 address 00:1B:21:3C:69:44
	
	ip link set up rkni_v21
	ip link set dev rkni_v21 address 00:1B:21:3C:69:44

### 4.4.2. Starting Quagga

Zebra and bpgd quagga's components should be configured and started in order the_router to be
able to cummunicate with others BPG routers.

Zebra's configuration file /etc/quagga/zebra.conf

	hostname h4
	
	! Set both of these passwords
	password xxx
	enable password xxx
	
	# rt1
	table 250
	
	! Turn off welcome messages
	no banner motd
	log file /var/log/quagga/zebra.log

The most important command in this file is the command "table 250". It tells Zebra
to install received routes into an additional linux routing table with id 250.
Id of this tables should be added to /etc/iproute2/rt_tables

	250     rt1

Using a separate routing table is requied so the received routes do not mess with routes linux uses
in its standard network operations. The routes quagga receives via KNI interfaces and then install to the table 'rt1'
are meant only for the_router and have no value for the linux host. The zebra will pass that routes to the_router
via "zebra FIB push interface" after they have been received and installed into the table 'rt1'.

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

A single bgp peer with the uplink border router 192.168.1.3 is described in this configuration file. 
The router announces network 10.111.0.0/29 which is used in SNAT function and serve as a pulic
addresses of the_router's subscribers.
Note that a private prefix 10.111.0.0/29 is used only due the test nature of this howto and lack of public/routed
ip address. In a real environment a routed/public ip addresses usally are used in the cases like this.

Run bgpd

	/etc/init.d/bgpd start
	
Insure that default (0.0.0.0/0) route is successfully received and installed into both the linux route table and the_router's route table.

Linux routing table 'rt1' (id 250)

	h4 src # ip route ls table rt1
	default via 192.168.1.3 dev rkni_v3  proto zebra  metric 20

The TheRouter's main routing table:

	h4 src # rcli sh ip route
	224.0.0.0/4 is unreachable
	192.168.21.0/24 C dev v21 src 192.168.21.1
	192.168.5.0/24 C dev v5 src 192.168.5.1
	192.168.20.0/24 C dev v20 src 192.168.20.1
	192.168.1.0/24 C dev v3 src 192.168.1.112
	10.10.0.0/24 is unreachable
	0.0.0.0/0 via 192.168.1.3 dev v3 src 192.168.1.112

# 5. Radius configuration

## 5.1. TheRouter radius client configuration

Following commands define RADIUS server ip address,
source ip address of RADIUS router's requests,
shared radius secret.

	radius_client add server 192.168.20.2
	radius_client add src ip 192.168.20.1
	radius_client set secret "secret"

The source ip address must be assigned to the router's interface which
connects a router to a RADIUS server. It is a v20 interface in this
test lab, since it connects the_router with the Linux host H4 
where the RADIUS server is running.

	# link with local linux host
	vif add name v20 port 0 type dot1q cvid 20
	ip addr add 192.168.20.1/24 dev v20

Ip address 192.168.20.1 is configured on the linux side of vlan 20.

	9: vlan20@eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
	    link/ether 90:e2:ba:4b:b3:17 brd ff:ff:ff:ff:ff:ff
	    inet 192.168.20.2/24 brd 192.168.20.255 scope global vlan20
	       valid_lft forever preferred_lft forever

## 5.2. Radius server configuration 

FreeRadius is used as a RADIUS server in this howto.
The FreeRadius project has a very good documentations and there are a lot of configuration
examples available on the internet, so, excuse me for not providing any examples here.

But, I will provide the text of the main SQL query that illustrates using of TheRouter specific
radius attributes and the text of TheRouter VAS dictionary is also provided.

/etc/raddb/sql/mysql/dialup.conf

	## router_bras_dhcp_relay.conf
	## pbr.
	authorize_reply_query = "SELECT 1, '%{SQL-User-Name}', 'therouter_ingress_cir', '200', '=' \
	UNION SELECT 2, '%{SQL-User-Name}', 'therouter_engress_cir', '200', '+=' \
	UNION SELECT 3, '%{SQL-User-Name}', 'therouter_ipv4_addr', GetIpoeUserService(%{request:therouter_port_id}, '%{request:therouter_outer_vid}', '%{request:therouter_inner_vid}'), '+=' \
	UNION SELECT 4, '%{SQL-User-Name}', 'therouter_ipv4_mask', '24', '+=' \
	UNION SELECT 5, '%{SQL-User-Name}', 'therouter_ip_unnumbered', '1', '+='"

This SQL query is used by FreeRadius to query the data required to form a radius response
to a router's radius request to authorize a subscriber connected via a dedicated vlan.
Mysql stored procedure GetIpoeUserService will calculate subscriber's ip address
based on subscriber's vlan id and port number via it connected to the_router.
The information provided in a radius reply will be used then by the_router to configure
subscribers routing using according to the ip unnumbered rules.

The detailed description of the ip unnumberes rules is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber">Vlan per subscriber</a>

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
	END-VENDOR   TheRouter

# 6. Configure subscriber's sessions or dynamic VIF

## 6.1. Dynamic "Vlan per subscriber" subscriber's interfaces

The detailed description of "vlan per subscriber" dynamic interfaces is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber">vlan-per-subscriber</a>.
I am going to briefly describe them according to the context of our test lab.

Dynamic interfaces (dynamic VIF) are created automatically by the_router in a response
to user traffic coming to a router port. They cannot be created by a user via rcli interface 
or by a configuration file command. In order the router to create a dynamic VIF it should
be informed on which port number it should expect a user traffic that will trigger a dynamic VIF creation.

	port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 1,2

When the_router receives on a port a packet that doesn't match any known VIF, the router
creates a new dynamic interface provided that creation of the interfaces is allowed by the RADIUS
protocol. Vlan id values for a new virtual interface will be gathered from the packet's ethernet header.

RADIUS request to authorize a creation of new dynamic VIF will contain data about subsriber's vlan.
RADIUS response should contain the data requied to configure IP protocol for a new dynamic VIF and IP forwarding in
the case of ip unnumbered.

### 6.1.1. Viewing ip subscrubers 

	h4 ~ # rcli sh subsc
	vlan    subsc ip        mode    port    ingress_car     egress_car      rx_pkts tx_pkts
		...
	0.130                   1       0       200 mbit/s      200 mbit/s      0       0
		...

The "sh subsc" command also ouputs a list of L2/L3 subscriber sessions, therefore some field values may be blank.
For example, a dynamic VIF "subsc ip" field is blank.

### 6.1.2. Dynamic vif routing

When a radius response to dynamic VIF creation request includes radius VSA attribute "therouter_ip_unnumbered",
TheRouter creates a route for a subscriber. Detailed description of this process is provided in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#Ответ-на-запрос-авторизации">
Radius Authorization responce</a>.

Insure that a route has been created:

	h4 ~ # rcli sh ip route
	...
	10.10.0.11/32 C dev dvif0.130 src 10.10.0.1
	...

10.10.0.11 is a ip address value included in a radius reply.
dvif1.0.130 is a name of a dynamic interface. "1.0.130" part of a name is a concatenation of values of the fiels
port_id, svid, cvid

## 6.2. L2/L3 connected subscribers

### 6.2.1. L2/L3 subscribers
Detailed information about L2 subscriber sessions is located in 
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#l2-connected-subscribers">
L2 connected subscribers</a>, and L3 connected subscriber detailed descriptions is in the chapter
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#l3-connected-subscribers">
L3 connected subscribers</a>

Examples of our test BRAS L2/L3 subscribers sessions will be described below.

L2/L3 subscriber sessions are not a kind of VIF interface and they are not required any IP configuration steps 
such as a route creation or assigning an ip address to be executed on the router side.

L2/L3 subsriber sessions belong to a VIF, therefore in order to create them the special flag should added
to a vif creation command. To allow the router to create L2 subscriber sessions the "l2_subs" flag
should be used. To allow the router to create L3 subscriber sessions the "l3_subs" flag
should be used.

In our test lab L2 subscriber sessions are create on VIF v5:

	vif add name v5 port 0 type dot1q cvid 5 flags kni,l2_subs

L3 sessions are created on VIF v21:

	vif add name v21 port 0 type dot1q cvid 21 flags kni,l3_subs	

The only difference between L2 and L3 subscriber sessions is that L2 sessions store
subscriber's MAC address in addition to subscriber's ip address. Storing a MAC address 
allows the_router (not implemented yet) to make sure that all packets going throw a session match the session MAC address,
and to execute defined actions when a packet doesn't match a session.

Authorization of creation L2/L3 sessions works the same way as the authorization process of dynamic VIFs,
but it doesn't require any IP configuration steps (assigning an ip address, creation an ip route).

L2/L3 sessions support shaping of traffic going through a session.

### 6.2.2. Viewing L2/L3 sessions

	h4 ~ # rcli sh subsc
	vlan    subsc ip        mode    port    ingress_car     egress_car      rx_pkts tx_pkts
	        192.168.5.132   1       0       200 mbit/s      200 mbit/s      0       0
	        192.168.5.133   1       0       200 mbit/s      200 mbit/s      0       0

# 7. DHCP server and DHCP Relay configuration

TheRouter software supports DHCP Relay feature.
The only configure option available so far is the ip address of a DHCP server
to relay DHCP requests to.

	dhcp_relay 192.168.20.2

Ip address 192.168.20.2 is the address of H4 host where a DHCP server is running.

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

In order dhcp server responses to successfully reach their destinations
a dhcp server should know a route to dhcp relay server requests are coming from.
In our example, dhcp requests are coming from ip address 10.10.0.1. This is the address
assigned to all dynamic interfaces according to ip unnumbered scheme. TheRouter relays 
dhcp requests of subsribers connected via dynamic interfaces and alters those requests,
adding ip address 10.10.0.1 to them.

The following dhcpd log files entries illustares described scheme.

	Aug 23 21:07:24 h4 dhcpd[2888]: DHCPREQUEST for 10.10.0.11 (192.168.20.2) from 00:88:65:36:39:4c via 10.10.0.1
	Aug 23 21:07:24 h4 dhcpd[2888]: DHCPACK on 10.10.0.11 to 00:88:65:36:39:4c via 10.10.0.1

The end of each log entry "via 10.10.0.1" indicates from which server a request was received.

Therefore there should be a route to the destination 10.10.0.1 on linux host H4.
The route should point to TheRouter which is connected via vlan 20, so the route 
should be added by the following command:

	ip route add 10.10.0.1 via 192.168.20.1

# 8. Configuration of the redirecting unauthorized users traffic to a site

Traffic redirection is implemented via PBR mechanism (Policy based routing) and described in
the <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#Управление-трафиком-заблокированных-подписчиков">Control traffic of unauthorised user</a>

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
	8 is calculated like this: 32 - 29 == 3 ^ 2 == 8

According to that rule the same source ip address will always be tranlated to the same address from the defined address pool.
