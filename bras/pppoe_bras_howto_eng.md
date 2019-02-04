# 1. Introduction

This howto describes configuration of a PPPoE BRAS server running TheRouter software
under Linux operating system.

The PPPoE BRAS server is going to be integrated into a small test network and it should accomplish the following
tasks:

 * termination of subscriber's PPPoE tunnels;
 * subscriber authorization and authentication via RADIUS protocol;
 * redirection unauthorized subscriber's traffic to a WEB server;
 * shaping subscriber's traffic;
 * forwarding traffic based on routes received via BGP protocol;
 * SNAT - network adress translation of users source private ip addresses into a pool of public/white
 ip addresses;
	
# 2. Configuration Linux and TheRouter software

Follow either <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo installation</a>
instructions or <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu installation</a>
instruction and install DPDK and TheRouter on a x86-64 server running Gentoo Linux or Ubuntu.
A server should be using at least Intel core family processors or similar.

# 3. Test network scheme


# 4. Starting TheRouter, checking L2 and L3 connectivity.

## 4.1. Starting TheRouter

	ip netns add tr
	export rvrf="ip netns exec tr"
	$rvrf ip link set up lo
	
	ip netns exec tr the_router \
	 --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' \
	 -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -d -c /etc/router_pppoe.conf
	
	sleep 5
	$rvrf ip link set up dev rkni_v3 address 6C:B3:11:51:15:50

## 4.2. TheRouter's configuration file

Here is the full router's configuration file.
Detailed description of the configuration commands will be provided in the following paragraphs.

	startup {
	  # mbuf mempool size
	  sysctl set mbuf 8192
	
	  # note: RSS mode rss_l2_src_dst is supported only on Intel X710 series NICs
	  port 0 mtu 1500 tpid 0x8100 state enabled rss rss_l2_src_dst
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
		
	  sysctl set log_level 8
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 600
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	
	  sysctl set flow_acct 0
	  sysctl set flow_acct_dropped_pkts 0
	  sysctl set nd_neighbor_cache_entry_ttl 120
	
	  # PPPoE
	  sysctl set pppoe_max_subsc 50000
	  sysctl set subsc_vif_max 50000
	  sysctl set radius_max_sessions 20000
	  sysctl set pppoe_sub_uniq_check 1
	
	  # any protocol timeouts (UDP)
	  sysctl set NPF_ANY_CONN_CLOSED 2
	  sysctl set NPF_ANY_CONN_NEW 30
	  sysctl set NPF_ANY_CONN_ESTABLISHED 60
	  
	  # TCP timeouts
	  sysctl set NPF_TCPS_CLOSED 10
	  sysctl set NPF_TCPS_SYN_SENT 30
	  sysctl set NPF_TCPS_SIMSYN_SENT 30
	  sysctl set NPF_TCPS_SYN_RECEIVED 60
	  sysctl set NPF_TCPS_ESTABLISHED 600
	  sysctl set NPF_TCPS_FIN_SENT 240
	  sysctl set NPF_TCPS_FIN_RECEIVED 240
	  sysctl set NPF_TCPS_CLOSE_WAIT 45
	  sysctl set NPF_TCPS_FIN_WAIT 60
	  sysctl set NPF_TCPS_CLOSING 30
	  sysctl set NPF_TCPS_LAST_ACK 30
	  sysctl set NPF_TCPS_TIME_WAIT 120
	}
	
	runtime {
	
	  flow ipfix_collector addr 192.168.1.73
	
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
		
	  vif add name v3 port 0 type dot1q cvid 3 flags flow_acct,pppoe_on,npf_on
	  vif add name v5 port 0 type dot1q cvid 5 flags flow_acct
	
	  arp add 192.168.1.73 60:a4:4c:41:0a:24 dev v3 static
	
	
	  ip addr add 192.168.1.111/24 dev v3
	  ip addr add 192.168.1.118/24 dev v3
	
	  ip addr add 192.168.5.111/24 dev v5
	  # ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.111
	
	  ipv6 enable dev v3
	  ipv6 addr add 2001:470:1f0b:1460::10/64 dev v3
	
	  ipv6 enable dev v5
	  ipv6 addr add 2001:470:1f0b:1460::5:2/112 dev v5
	  ipv6 nd ra enable dev v5
	
	  # default route
	  ipv6 route add ::/0 via 2001:470:1f0b:1460::1
	
	  ipv6 addr add 2001:470:1f0b:1460::6:2/112 dev v5
	
	  #
	  # PPPoE subscribers
	  #
	  ip route table add rt_bl
	  u32set create ips1 size 16384 bucket_size 16
	  u32set create l2s1 size 16384 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  # pbr rules
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl
	
	  # pppoe
	  	  
	  # enable TCP MSS fix
	  sysctl set tcp_mss_fix 1
	  
	  pppoe ac_cookie key "13071232717"
	  pppoe ac_name "trouter1"
	  pppoe service name "*"
	
	  # ppp
	  ppp dns primary 8.8.8.8
	  ppp dns secondary 8.8.4.4
	  ppp ipcp server ip 10.10.1.1
	
	  # pppoe p-t-p address
	  ip route add 10.10.1.1/32 local
	
	  #
	  # radius
	  #
	  radius_client add src ip 192.168.5.111
	  radius_client add server 192.168.5.2
	  radius_client set secret "secret"
	  coa server set secret "secret"
	
	  #
	  # ACL
	  #
	  vif acl create aclid 10 type ipv6_tuple permit
	  vif acl rule ipv6 add aclid 10 prio 20 dst 2a02:6b8::2:242 dport 80
	  vif acl add dev v5 dir ingress aclid 10 prio 30
	
	  vif acl create aclid 11 type ipv6_tuple deny
	  vif acl add dev v5 dir ingress aclid 11 prio 31
	
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}

## 4.3. Start KNI interfaces and Quagga/FRR

Detailed description of a way TheRouter communicates with Quagga/FRR is provided on the page 
<a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite">
Dynamic routing. Integration with Quagga routing suite
</a>

### 4.3.1. KNI interfaces

a KNI interface must be created for each router's VIF that will be used to communicate
with other routers via dynamic routing protocols supported by Quagga/FRR. Example of a such interface
is the v3 interface. The router receives a default route from the bgp peer established via the v3 kni interface.

You should up KNI interfaces and configure their MAC addresses after the_router has started.
MAC address of a kni interface should be equal to the mac address of the router's interface (VIF) 
coupled with a kni interface. MAC address of a VIF can be found out from the 'rcli sh vif' command output.

Example of a bash script that up a kni interface and configure it's MAC address:

	#!/bin/bash
	ip link set up dev rkni_v3 address 00:1B:21:3C:69:44
	
### 4.3.2. Starting FRR

Zebra and bpgd Quagga/FRR components should be configured and started in the router namespace
"tr" in order the_router to cummunicate with others BPG peers.

Zebra's configuration file /etc/frr/zebra.conf

	hostname h5
	
	! Set both of these passwords
	password xxx
	enable password xxx
		
	! Turn off welcome messages
	no banner motd
	log file /var/log/frr/zebra.log


Run zebra

	/etc/init.d/zebra start

Once zebra has stated, run bgpd.

Bgpd's configuration file /etc/frr/bgpd.conf

	!
	! Zebra configuration saved from vty
	!   2018/11/14 18:26:20
	!
	frr version 4.0
	frr defaults traditional
	!
	hostname h5
	password xxx
	log syslog
	!
	!
	router bgp 64512
	 bgp router-id 192.168.1.111
	 network 10.114.0.0/29
	 neighbor 192.168.1.3 remote-as 64513
	exit
	!
	!
	line vty
	!

A single bgp peer with the uplink border router 192.168.1.3 is described in this configuration file. 
The router announces network 10.114.0.0/29 which is used in SNAT function and serves as a pulic
addresses for pppoe subscribers.

Note that a private prefix 10.114.0.0/29 is used only due the test nature of this howto and lack of global public
ip addresses. In a real environment global public ip addresses are usally used in the similar cases.

Run bgpd

	/etc/init.d/bgpd start
	
Insure that a default (0.0.0.0/0) route is successfully received and installed into 
both the linux route table and the_router's route table.

Linux routing table in the "tr" namespace

	h5 src # $rvrf ip route ls
	default via 192.168.1.3 dev rkni_v3 proto 186 metric 20
	192.168.1.0/24 dev rkni_v3 proto kernel scope link src 192.168.1.111

The TheRouter's main routing table:

	h5 src # $rvrf rcli sh ip route
	192.168.5.0/24 C dev v5 src 192.168.5.111
	5.5.5.5/32 local
	192.168.1.0/24 C dev v3 src 192.168.1.118
	10.10.1.1/32 local
	224.0.0.0/4 unreachable
	0.0.0.0/0 via 192.168.1.3 dev v3 src 192.168.1.118

## 4.4. Connectivity checking 

### 4.4.1. Insure that the interfaces described in the configuration file are up and running:

	h5 src # $rvrf rcli sh vif
	vif v5, id 3
	  port 0, vlan 0.5, encapsulation dot1q
	  mac address 6C:B3:11:51:15:50
	  NPF index 12
	  CAR ingress not set
	      egress not set
	  ACL ingress prio 30 acl 10, prio 31 acl 11
	      egress not set
	  flags FLOW
	  mtu 1500
	vif v3, id 2
	  port 0, vlan 0.3, encapsulation dot1q
	  mac address 6C:B3:11:51:15:50
	  NPF index 11
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags NPF,FLOW,pppoe
	  mtu 1500
	vif lo, id 1
	  port 255, vlan 4095.4095, encapsulation qinq
	  mac address 00:00:00:00:00:01
	  NPF index 10
	  CAR ingress not set
	      egress not set
	  ACL ingress not set
	      egress not set
	  flags none
	  mtu 1500

### 4.4.2. ARP

	h5 ~ # $rvrf rcli sh arp
	port    vid     ip      mac     type    state
	0       0.3     192.168.1.73    60:A4:4C:41:0A:24       static  ok

### 4.4.3. ICMP

	h5 ~ # $rvrf rcli ping 192.168.1.3
	Ping 192.168.1.3 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.209 ms
	reply 56 bytes icmp_seq=2 time=0.206 ms
	reply 56 bytes icmp_seq=3 time=0.206 ms
	reply 56 bytes icmp_seq=4 time=0.208 ms
	reply 56 bytes icmp_seq=5 time=0.336 ms
	---
	Ping 192.168.1.3 statistics:
	sent: 5, recv: 5 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.206/0.232/0.336

# 5. Radius configuration

## 5.1. TheRouter radius client configuration

Following commands define RADIUS server ip address,
source ip address of RADIUS router's requests,
shared radius and CoA secrets.

	radius_client add src ip 192.168.5.111
	radius_client add server 192.168.5.2
	radius_client set secret "secret"
	coa server set secret "secret"

The source ip address must be assigned to the router's interface which
connects the router with a RADIUS server. It is the v5 interface, 
since it connects the_router to the linux host H5 
where the RADIUS server is running.

Ip address 192.168.5.2 is configured on the linux side of vlan 5.

	h5 src # ip addr ls dev vlan5
	16: vlan5@eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
	    link/ether 60:a4:4c:41:0a:24 brd ff:ff:ff:ff:ff:ff
	    inet 192.168.5.2/24 brd 192.168.5.255 scope global vlan5
	       valid_lft forever preferred_lft forever
	    inet6 fe80::62a4:4cff:fe41:a24/64 scope link
	       valid_lft forever preferred_lft forever

## 5.2. Radius server configuration 

FreeRadius is used as a RADIUS server in this howto.
The FreeRadius project has a very good documentations and there are a lot of configuration
examples available on the internet, so, excuse me for not providing any examples here.

But, I will provide the text of the main SQL query that illustrates using of TheRouter specific
radius attributes and I will also provide the text of TheRouter VAS dictionary.

/etc/raddb/sql/mysql/dialup.conf

	## router_bras_dhcp_relay.conf
	## router pppoe bras
	## 168493313 - 10.11.1.1
	authorize_reply_query = "SELECT 1, '%{SQL-User-Name}', 'therouter_ingress_cir', '100', '=' \
	UNION SELECT 2, '%{SQL-User-Name}', 'therouter_egress_cir', '100', '+=' \
	UNION SELECT 3, '%{SQL-User-Name}', 'therouter_ipv4_addr', GetPppoeSubscIp('%{SQL-User-Name}'), '+=' \
	UNION SELECT 4, '%{SQL-User-Name}', 'therouter_ipv4_mask', '32', '+='"

This SQL query is used by FreeRadius to query the data required to form a radius response
to a router's radius request to authorize a subscriber connected via pppoe.
Mysql stored procedure GetIpoeUserService calculates subscriber's ip address
based on subscriber's login.
The information provided in a radius reply then will be used by the_router to configure
a subscriber pppoe interface.

therouter_egress_cir and therouter_egress_cir attributes should contain ingress and
egress CIR values in kbit/s.

therouter_ipv4_addr attribute should contain IPv4 address in numeric format.

### 5.2.2. FreeRadius dictionary

Add the following lines to the /etc/raddb/dictionary

	VENDOR       TheRouter     12345
	BEGIN-VENDOR TheRouter
	    ATTRIBUTE therouter_ingress_cir 1 integer
	    ATTRIBUTE therouter_egress_cir 2 integer
	    ATTRIBUTE therouter_ipv4_addr 3 integer
	    ATTRIBUTE therouter_ipv4_mask 4 integer
	    ATTRIBUTE therouter_outer_vid 5 integer
	    ATTRIBUTE therouter_inner_vid 6 integer
	    ATTRIBUTE therouter_ip_unnumbered 7 integer
	    ATTRIBUTE therouter_port_id 8 integer
	    ATTRIBUTE therouter_ipv4_gw 9 integer
	    ATTRIBUTE therouter_pbr 10 integer
	END-VENDOR   TheRouter

# 6. PPPoE and PPP

### 6.1.1. Enable PPPoE on a VIF

First enable PPPoE protocol on a VIF router's interface connected to a L2 subnet
with pppoe subscribers

	vif add name v3 port 0 type dot1q cvid 3 flags flow_acct,pppoe_on,npf_on

Then, configure ip address of the router side of pppoe p-t-p tunnels
and create a local route to that address:

	ppp ipcp server ip 10.10.1.1
	
	# pppoe p-t-p address
	ip route add 10.10.1.1/32 local

Configure ppp subscribers ip parameters:

	# ppp
	ppp dns primary 8.8.8.8
	ppp dns secondary 8.8.4.4

Enable TCP MSS fix:

	# enable TCP MSS fix
	sysctl set tcp_mss_fix 1

Configure PPPoE parameters:

	pppoe ac_cookie key "13071232717"
	pppoe ac_name "trouter1"
	pppoe service name "*"

Now everything is ready to connect pppoe subscribers.

### 6.1.1. Viewing ip subscrubers 

	h5 src # $rvrf rcli sh pppoe subsc
	vif_id  mac     session_id      ip addr mtu     ingress cir     egress cir      tx_pkts rx_pkts
	4       60:A4:4C:41:0A:24       1       0.0.0.0 1480    0       0       0       0

Since log_level 8 is configured there are a lot of ppp debug info the syslog file:

Establishing a pppoe connection:

	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: OPEN event
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Initial - Starting
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: OPEN event
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Initial - Starting
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: UP event
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Starting: send a configure request #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Starting - Req sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Req sent: receive a configure request #3
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Req sent - Req sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Req sent: received a configure Nak/Reject code 3, #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Req sent: send a configure request #2
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Req sent - Req sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Req sent: receive a configure request #4
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Req sent: send a configure ack #4
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Req sent - Ack sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Ack sent: received a configure Ack #2
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Ack sent - Opened
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH PAP, subsc VIF id 7, state initial: start AUTH process
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH, subsc VIF id 7, state transition initial -- waiting for peer's request
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH PAP, subsc VIF id 7, state waiting for peer's request: received PAP request #3
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH, subsc VIF id 7, state transition waiting for peer's request -- peer request received
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH PAP, subsc VIF id 7, state: peer request received, send a result code to the peer: code 2, id #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH, subsc VIF id 7, state transition peer request received -- finished
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: PPP AUTH, auth proto 49187, subsc VIF id 7, state finished, timer 0x7f4f742e72c0, auth ctx fini
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: UP event
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Starting: send a configure request #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Starting - Req sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Req sent: receive a configure request #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Req sent - Req sent
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Req sent: received a configure Ack #1
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Req sent - Ack rcvd
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Ack rcvd: receive a configure request #2
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Ack rcvd - Ack rcvd
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Ack rcvd: receive a configure request #3
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Ack rcvd: send a configure ack #3
	Jan 24 19:06:20 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Ack rcvd - Opened

Closing a pppoe connection:

	$rvrf rcli pppoe close 7


	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: Close event
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Opened: send a terminate request #3
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Opened - Closing
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7, state Closing: received a terminate Ack #3
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Closing - Closed
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: IPCP, subsc VIF id 7: shutdown, send close event message to LCP
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: Close event
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: Down event
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: state transition: Closed - Initial
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM IPCP, subsc VIF id 7: Close event
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: IPCP, subsc VIF id 7: shutdown, send close event message to LCP
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Opened: send a terminate request #4
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Opened - Closing
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: Close event
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Closing - Closing
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7, state Closing: received a terminate Ack #4
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: state transition: Closing - Closed
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: FSM LCP, subsc VIF id 7: shutdown: admin shutdown (6)
	Jan 24 19:07:49 h5 the_router[19078]: ROUTER: lcore 0 shutdown PPPoE: port 0, session id 4, 4bmac 1279330852, reason 10252

# 7. CoA

### 7.1 Add subsriber's ip address to ip set of blocked/unuthorised users.

	echo User-Name=pppoe5,User-Password=mypass,Vendor-Specific = "TheRouter,therouter_pbr=1" | radclient 192.168.5.111:3799 coa secret

IP set containing blocked subscribers ip addresses is defined by command:

	u32set create ips1 size 16384 bucket_size 16

That IP set is used by PBR rules to redirect blocked/unuthorised pppoe users to a direrect routing table:

	# pbr rules
	ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl

### 7.2 Remove subsriber's ip address from ip set of blocked/unuthorised users.

	echo User-Name=pppoe5,User-Password=mypass,Vendor-Specific = "TheRouter,therouter_pbr=2" | radclient 192.168.5.111:3799 coa secret

### 7.2 Change traffic shaping parameters of PPPoE subsriber

	echo User-Name=pppoe9,User-Password=mypass,Vendor-Specific = "TheRouter,therouter_ingress_cir=50,therouter_engress_cir=55" | radclient 192.168.5.111:3799 coa secret

# 8. NAT configuration

NAT configuration is conntrolled by a separate configuration file which is defined by the following command:

	npf load "/etc/npf.conf.bras_dhcp_relay"

File /etc/npf.conf.bras_dhcp_relay

	map v3 netmap 10.114.0.0/29
	
	group default {
	  pass stateful final on v3 all
	}
	
Command "map v3 netmap 10.114.0.0/29" defines NAT translation of source IP addresses of packets going through 
v3 interface. Translation replaces a source ip address with an address from the pool 10.114.0.0/29.
The new address of a packet is calculated by combining an original packet source address and the prefix 10.114.0.0/29.

	new address is: 10.114.0.0 plus first 8 bits from an original address.
	Where 8 is calculated as: 32 - 29 == 3 ^ 2 == 8, where 29 - is taken from the netmap command.

According to that rule the same source ip address will always be tranlated to the same address from the defined address pool.
