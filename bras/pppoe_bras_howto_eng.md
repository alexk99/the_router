# 1. Introduction

This howto describes the configuration of a PPPoE BRAS server running TheRouter software
under the Linux operating system.

The PPPoE BRAS server is going to be integrated into a small test network and it should accomplish the following
tasks:

 * termination of subscriber's PPPoE tunnels;
 * subscriber authorization and authentication via RADIUS protocol;
 * redirection unauthorized subscriber's traffic to a WEB server;
 * shaping subscriber's traffic;
 * forwarding traffic based on routes received via BGP protocol;
 * SNAT - network address translation of users source private ip addresses into a pool of public/white
 ip addresses;
	
# 2. Configuration Linux and TheRouter software

Follow either <a href="https://github.com/alexk99/the_router/blob/master/install.md">Gentoo installation</a>
instructions or <a href="https://github.com/alexk99/the_router/blob/master/ubuntu_install.md">Ubuntu installation</a>
instruction and install DPDK and TheRouter on a x86-64 server running Gentoo Linux or Ubuntu.
A server should be using at least Intel core family processors or similar.

To download pppoe enabled build use the link on main page in the section PPPoE.

# 3. Test network scheme


# 4. Starting TheRouter, checking L2 and L3 connectivity.

## 4.1. Starting TheRouter

	# stop FRR/Quagga daemons
	fixme

	ip netns add tr
	export rvrf="ip netns exec tr"
	$rvrf ip link set up lo
	$rvrf ip route flush dev lo
	
	ip netns exec tr the_router \
	 --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' \
	 -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -d -c /etc/router_pppoe.conf
	
	sleep 5
	#
	# !! use your own MAC address of a physical port
	# !! use the 'rcli sh vif' command to figure out MAC addresses of ports
	#
	$rvrf ip link set up address 6C:B3:11:51:15:50 dev rkni_v3
	
	#
	# start FRR/Quagga daemons
	#
	fixme

## 4.2. TheRouter's configuration file

Here is the full router's configuration file.
The detailed description of the configuration commands will be provided in the following paragraphs.

	startup {
	  sysctl set numa 0
	
	  # mbuf mempool size
	  sysctl set mbuf 16384
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3

	  rx_queue port 1 queue 0 lcore 1
	  rx_queue port 1 queue 1 lcore 2
	  rx_queue port 1 queue 2 lcore 3
		
	  sysctl set log_level 7
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 600
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	  sysctl set frag_mbuf 4000
	  sysctl set mac_addr_format "linux"
	  
	  sysctl set pppoe_max_subsc 50000
	  sysctl set radius_max_sessions 20000
	  sysctl set subsc_vif_max 50000
	  sysctl set pppoe_sub_uniq_check 1
	  sysctl set ppp_1session_per_username 1
	  sysctl set ppp_install_subsc_linux_routes 1
	
	  #
	  # IP Pool cache size, TTL
	  #
	  # IP Pool serve the same purpose as DHCP lease database does
	  # and allows to have persistente ipv4 addresses as long as
	  # TheRouter doens't reboot (no filesystem storage)
	  #
	  sysctl set ippool_cache_size 4096
	  sysctl set ippool_cache_entry_ttl 172800
	
	  #
	  # System name
	  #
	  sysctl set system_name "tr1"
	  		  
	  #
	  # LPM DIR24-8 IPv4 FIB
	  #
	  sysctl set lpm_table8_size 2048
	
	  #
	  # 3 - RTPROT_BOOT (linux netlink routes proto) 
	  # Note: FRR 4.0 bgpd redistribute kernel doesn't see linux routes with proto static,
	  # but it sees BOOT routes
	  #
	  sysctl set linux_route_proto 3  
	}
	
	runtime {
	  # 
	  # NPF, NAT, 
	  # any protocol timeouts (UDP)
	  #
	  sysctl set NPF_ANY_CONN_CLOSED 2
	  sysctl set NPF_ANY_CONN_NEW 30
	  sysctl set NPF_ANY_CONN_ESTABLISHED 60
	
	  # 
	  # NPF, NAT
	  # TCP timeouts
	  #
	  sysctl set NPF_TCPS_CLOSED 10
	  sysctl set NPF_TCPS_SYN_SENT 30
	  sysctl set NPF_TCPS_SIMSYN_SENT 30
	  sysctl set NPF_TCPS_SYN_RECEIVED 60
	  sysctl set NPF_TCPS_ESTABLISHED 6000
	  sysctl set NPF_TCPS_FIN_SENT 240
	  sysctl set NPF_TCPS_FIN_RECEIVED 240
	  sysctl set NPF_TCPS_CLOSE_WAIT 45
	  sysctl set NPF_TCPS_FIN_WAIT 60
	  sysctl set NPF_TCPS_CLOSING 30
	  sysctl set NPF_TCPS_LAST_ACK 30
	  sysctl set NPF_TCPS_TIME_WAIT 120
	
	  #
	  # static IP routes
	  #
	  
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole NAT pool
	  ip route add 10.114.0.0/29 unreachable
	
	  #
	  # Interfaces
	  #
	  # uplink interface
	  vif add name v3 port 0 type dot1q cvid 3 flags npf_on,kni
	  
	  # interfaces to L2 domains with pppoe subscribers 
	  vif add name pppoe1 port 1 type dot1q cvid 101 flags pppoe_on
	  vif add name pppoe2 port 1 type dot1q cvid 102 flags pppoe_on
	  vif add name pppoe3 port 1 type dot1q cvid 103 flags pppoe_on
	  
	  # interface to a subnet with radius server
	  vif add name v5 port 0 type dot1q cvid 5
	
	  #
	  # Assigning IP addresses to interfaces
	  #
	  ip addr add 192.168.1.118/24 dev v3
	  ip addr add 192.168.5.111/24 dev v5
	
	  # 
	  # pppoe
	  #
	  sysctl set tcp_mss_fix 1
	  pppoe ac_cookie key "13071232717"
	  pppoe ac_name "trouter1"
	  pppoe service name "*"
		
	  # 
	  # ppp
	  #
	  ppp dns primary 8.8.8.8
	  ppp dns secondary 8.8.4.4
	
	  sysctl set ppp_max_configure 6
	  sysctl set ppp_initial_restart_time 500
	  
	  sysctl set lcp_keepalive_interval 30
	  sysctl set lcp_keepalive_num_retries 10
	  sysctl set lcp_keepalive_probe_interval 500
	  
	  #
	  # it's an IP address that will be assigned to each ptp pppoe interface
	  #
	  ppp ipcp server ip 10.10.1.1
	  ip route add 10.10.1.1/32 local
	
	  #
	  # IP pools
	  #
	  #
	  
	  # create IP pools
	  ip pool add ppp_pool_1
	  ip pool add range ppp_pool_1 10.11.1.10 - 10.11.100.250
	
	  ip pool add ppp_pool_2
	  ip pool add range ppp_pool_2 10.11.12.30 - 10.11.200.250
	  
	  # configure ppp to use ip pools 1 and 2
	  ppp ip pool add ppp_pool_1
	  ppp ip pool add ppp_pool_2
	  
	  #
	  # radius (authorization)
	  #
	  
	  # specify an addres assigned to the vif to the radius subnet (v5)
	  radius_client add src ip 192.168.5.111
	  	  
	  # 
	  # use your own ip of a radius server
	  #
	  radius_client add server 192.168.5.2 port 1812
	  radius_client set secret "xxx"
	  coa server set secret "xxx"
	  
	  #
	  # Radius accounting
	  #
	   
	  # replace 192.168.5.2 with an ip address of your radius server
	  #
	  radius_client add accounting server 192.168.5.2
	  radius_client set accounting secret "xx"

	  sysctl set radius_accounting 1
	  sysctl set radius_accounting_interim 1
	  sysctl set radius_accounting_interim_interval 600
	  sysctl set radius_initial_retransmit_timeout 500
	  
	  #
	  # PBR
	  #	  
	  ip route table add rt_bl
	  u32set create ips1 size 16384 bucket_size 16
	  u32set create l2s1 size 16384 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  # pbr rules
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl	
	
	  #
	  # Flow accounting
	  #
	  flow ipfix_collector addr 192.168.1.73
	  sysctl set flow_acct 1
	  sysctl set flow_acct_dropped_pkts 0	  
	
	  #
	  # NAT events (NSEL)
	  #
	  ipfix_collector addr 192.168.1.74
	  sysctl set ipfix_nat_events 1	
	  
	  #
	  # NPF (NAT)
	  #
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}	

## 4.3. Start KNI interfaces and Quagga/FRR

The detailed description of a way TheRouter communicates with Quagga/FRR is provided on the page 
<a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite">
Dynamic routing. Integration with Quagga routing suite
</a>

### 4.3.1. KNI interfaces

a KNI interface must be created for each router's VIF that will be used to communicate
with other routers via dynamic routing protocols supported by Quagga/FRR. Example of such interface
is the v3 interface. The router receives a default route from the BGP peer established via the v3 kni interface.

You should up KNI interfaces and configure their MAC addresses after the_router has started.
MAC address of a kni interface should be equal to the mac address of the router's interface (VIF) 
coupled with a kni interface. MAC address of a VIF can be found out from the 'rcli sh vif' command output.

Example of a bash script that up a kni interface and configure it's MAC address:

	#!/bin/bash
	ip link set up address 00:1B:21:3C:69:44 dev rkni_v3
	
### 4.3.2. Starting FRR

Zebra and bpgd Quagga/FRR components should be configured and started in the router namespace
"tr" in order the_router to communicate with other BPG peers.

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

A single BGP peer with the uplink border router 192.168.1.3 is described in this configuration file. 
The router announces network 10.114.0.0/29 which is used in SNAT function and serves as a public
addresses for PPPoE subscribers.

Note that a private prefix 10.114.0.0/29 is used only due to the test nature of this howto and lack of global public
ip addresses. In a real environment global public ip addresses are usually used in similar cases.

Run bgpd

	/etc/init.d/bgpd start
	
Ensure that a default (0.0.0.0/0) route is successfully received and installed into 
both the Linux route table and the_router's route table.

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
connects the router with a RADIUS server. It is the v5 interface since it connects the_router to the Linux host H5 
where the RADIUS server is running.

Ip address 192.168.5.2 is configured on the Linux side of vlan 5.

	h5 src # ip addr ls dev vlan5
	16: vlan5@eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
	    link/ether 60:a4:4c:41:0a:24 brd ff:ff:ff:ff:ff:ff
	    inet 192.168.5.2/24 brd 192.168.5.255 scope global vlan5
	       valid_lft forever preferred_lft forever
	    inet6 fe80::62a4:4cff:fe41:a24/64 scope link
	       valid_lft forever preferred_lft forever

## 5.2. Radius server configuration 

FreeRadius is used as a RADIUS server in this howto.
The FreeRadius project has a very good documentation and there are a lot of configuration
examples available on the internet, so, excuse me for not providing any examples here.

But, I will provide the text of the main SQL query that illustrates using of TheRouter specific
radius attributes and I will also provide the text of TheRouter VAS dictionary.

/etc/raddb/sql/mysql/dialup.conf

	## router_bras_dhcp_relay.conf
	## router pppoe bras
	## 168493313 - 10.11.1.1
	authorize_reply_query = "SELECT 1, '%{SQL-User-Name}', 'therouter_ingress_cir', '100000', '=' \
	UNION SELECT 2, '%{SQL-User-Name}', 'therouter_egress_cir', '100000', '+=' \
	UNION SELECT 3, '%{SQL-User-Name}', 'Framed-IP-Address', GetPppoeSubscIp('%{SQL-User-Name}'), '+='"

This SQL query is used by FreeRadius to query the data required to form a radius response
to a router's radius request to authorize a subscriber connected via pppoe.
Mysql stored procedure GetPppoeSubscIp calculates subscriber's ip address
based on subscriber's login.
The information provided in a radius reply then will be used by the_router to configure
a subscriber pppoe interface.

therouter_egress_cir and therouter_egress_cir attributes should contain ingress and
egress CIR values in kbit/s.

WISPr-Bandwidth-Max-Up/WISPr-Bandwidth-Max-Down attributes can be used instead
of the therouter_ingress_cir/therouter_egress_cir attributes. Unit of WISPr bandwidth
attriubes is bit/s.

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
	    ATTRIBUTE therouter_remote_id  11 string
	    ATTRIBUTE therouter_circuit_id 12 string
	    ATTRIBUTE therouter_lladdr     13 string	    
	END-VENDOR   TheRouter

# 6. PPPoE and PPP

### 6.1.1. Enable PPPoE on a VIF

First, enable PPPoE protocol on a VIF router's interface connected to an L2 subnet
with PPPoE subscribers

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

### 6.1.1. Viewing ip subscribers 

	h5 src # $rvrf rcli sh pppoe
	vif_id  username        mac     session_id      ip addr mtu     ingress cir     egress cir      rx_pkts tx_pkts rx_bytes        tx_bytes        uptime
	5       p2      60:A4:4C:41:0A:24       2       10.11.1.3       1440    200000  100000  0       0       0       0       1 hour(s), 21 min(s), 1 sec(s)
	7       alexk   84:16:F9:BD:54:F7       4       10.11.1.1       1472    200000  100000  0       0       0       0       0 hour(s), 3 min(s), 43 sec(s)

Since log_level 8 is configured there are a lot of ppp debug info the syslog file:

Establishing a pppoe connection:

	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: PPPoE discovery request: port 0, lladdr 84:16:F9:BD:54:F7
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 OPEN event
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Initial -> Starting
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 OPEN event
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Initial -> Starting
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 UP event
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Starting: send a configure request #1
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Starting -> Req sent
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Req sent: receive a configure request #1
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Req sent: send a configure ack #1
	Mar 29 18:31:48 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Req sent -> Ack sent
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Ack sent: send a configure request #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Ack sent -> Ack sent
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Ack sent: received a configure Ack #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Ack sent -> Opened
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH CHAP subsc 4, mac 84:16:F9:BD:54:F7 state initial: start AUTH process
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH subsc 4, mac 84:16:F9:BD:54:F7 state transition initial -> challenge sent
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH CHAP subsc 4, mac 84:16:F9:BD:54:F7 state challenge sent: send a challenge #1 to the peer
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH CHAP subsc 4, mac 84:16:F9:BD:54:F7 state challenge sent: receive response #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH subsc 4, mac 84:16:F9:BD:54:F7 state transition challenge sent -> peer's response received
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH CHAP subsc 4, mac 84:16:F9:BD:54:F7 state peer's response received, send a result code to the peer: code 3, id #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH subsc 4, mac 84:16:F9:BD:54:F7 state transition peer's response received -> finished
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: PPP AUTH subsc 4, mac 84:16:F9:BD:54:F7 username alexk, auth proto CHAP, state finished, result 1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 UP event
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Starting: send a configure request #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Starting -> Req sent
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Req sent: receive a configure request #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Req sent -> Req sent
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Req sent: received a configure Ack #1
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Req sent -> Ack rcvd
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Ack rcvd: receive a configure request #2
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Ack rcvd: send a configure ack #2
	Mar 29 18:31:49 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Ack rcvd -> Opened

Closing a pppoe connection:

	$rvrf rcli pppoe disconnect 4

	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 Close event
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Opened: send a terminate request #3
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Opened -> Closing
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state Closing: received a terminate Ack #3
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Closing -> Closed
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: IPCP subsc 4, mac 84:16:F9:BD:54:F7 shutdown, send close event message to LCP
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 Close event
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 Down event
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Closed -> Initial
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM IPCP subsc 4, mac 84:16:F9:BD:54:F7 Close event
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: IPCP subsc 4, mac 84:16:F9:BD:54:F7 shutdown, send close event message to LCP
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Opened: send a terminate request #164
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Opened -> Closing
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 Close event
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Closing -> Closing
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state Closing: received a terminate Ack #164
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 state transition: Closing -> Closed
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: FSM LCP subsc 4, mac 84:16:F9:BD:54:F7 shutdown: admin shutdown (6)
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: PPPoE shutdown: port 0, subsc 4, mac 84:16:F9:BD:54:F7, session 1, username alexk, 4bmac 4189934839, reason 6
	Mar 29 19:52:02 h5 the_router[7701]: ROUTER: PPPoE shutdown timer: subsc 4, mac 84:16:F9:BD:54:F7, session 1, port 0


# 7. CoA

### 7.1 CoA Subsriber Identification Attributes

To identify a pppoe subscriber the following radius attributes could be used:
User-Name, Acct-Session-ID.

To use User-name attribute as a subscriber's identification sysctl variable "ppp_1session_per_username"
must be turned on.

### 7.2 Add subscriber's ip address to the ip set containing blocked/unauthorized subscriber's ip addresses

	echo User-Name=user1,Vendor-Specific="TheRouter,therouter_pbr=1" | radclient 192.168.5.111:3799 coa secret

IP set containing blocked subscribers ip addresses is defined by the command:

	u32set create ips1 size 16384 bucket_size 16

That IP set is used by the PBR rules to redirect blocked/unauthorized 
pppoe subscribers to a special routing table named 'rt_bl':

	# pbr rules
	ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl

### 7.3 Remove subscriber's ip address from ip set with blocked/unauthorized subscriber's ip addresses

	echo User-Name=user1,Vendor-Specific="TheRouter,therouter_pbr=2" | radclient 192.168.5.111:3799 coa secret

### 7.4 Change traffic shaping parameters of PPPoE subscriber

	echo User-Name=user1,Vendor-Specific="TheRouter,therouter_ingress_cir=201000,therouter_egress_cir=202000" | radclient 192.168.5.111:3799 coa secret

or

	echo User-Name=user1,WISPr-Bandwidth-Max-Down=111000000,WISPr-Bandwidth-Max-Up=112000000 | radclient 192.168.5.111:3799 coa secret


WISPr-Bandwidth-Max-Down and WISPr-Bandwidth-Max-Up attributes set the shaping using bit/s as a unit.
therouter_ingress_cir and therouter_egress_cir use kbit/s as a unit and use traffic directions from
a router point of view.

# 8. NAT configuration

NAT configuration is conntrolled by a separate configuration file which is defined by the following command:

	npf load "/etc/npf.conf.bras_dhcp_relay"

File /etc/npf.conf.bras_dhcp_relay

	map v3 netmap 10.111.0.0/29 from 192.168.5.0/24
	map v3 netmap 10.111.0.0/29 from 10.10.0.0/24
	
	alg "icmp"
	alg "pptp"
	
	group default {
 		block stateful final on v3 proto 47 all
 		pass stateful final on v3 all
	}
	
Command "map v3 netmap 10.111.0.0/29" defines NAT translation of source IP addresses of packets going through 
v3 interface. Translation replaces a source ip address with an address from the pool 10.111.0.0/29.
The new address of a packet is calculated by combining an original packet source address and the prefix 10.111.0.0/29.

	new address is: 10.111.0.0 plus first 8 bits from an original address.
	Where 8 is calculated as: 32 - 29 == 3 ^ 2 == 8, where 29 - is taken from the netmap command.

According to that rule, the same source ip address will always be translated to the same address from the defined address pool.

# 9. ACL

Create a special kind of interface that will be used as a template interface
to configure ACL for pppoe subscribers. ACL configuration of the template
interface will be copied to every pppoe subscriber interface that will
have been created after.

	acl template vif add pppoe_acl_template_t1

Create an ACL

	# create ACL ruleset
	vif acl create aclid 5 type ipv4_tuple deny
	vif acl rule ipv4 add aclid 5 prio 10 src 172.17.0.0/16 dst 172.17.0.0/16

Assign the created ACL with id 5 to the pppoe ACL template interface

	vif acl add dev pppoe_acl_template_t1 dir ingress aclid 5 prio 10
	vif acl add dev pppoe_acl_template_t1 dir egress aclid 5 prio 10

Indicate the pppoe engine to use the configured ACL template interface with
name "pppoe_acl_template_t1"

	sysctl set pppoe_template_acl "pppoe_acl_template_t1"

From now on, all PPPoE interfaces that have been created after this command
are going to use ACL configured on the template interface pppoe_acl_template_t1.
All ACL changes to pppoe_acl_template_t1 will be used by PPPoE interfaces.

Detailed description of ACL commands are provided <a href="https://github.com/alexk99/the_router/blob/master/conf_options.md#access-control-lists-acl">here</a> 