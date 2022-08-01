# IPoE IPv4 BNG example configuration.
  
In the following example 10.10.0.0/16 is a network with IPoE subscribers
and ip unnumbered scheme is used to configure IPv4 subscriber's routing.

Subscribers VLANs created automatically at port 1.
Each VLAN should connect only a single subscriber device.

Note that some ipv6 values marked as XX in the IPv6 section should be changed to your actual 
ipv6 prefix and addresses.

	startup {
		sysctl set numa 0

		# mbuf mempool size
		# this value depends on the total number of rx queues
		# mbuf = number of rx queues * 4096
		sysctl set mbuf 32000

		# NIC rx/tx descriptor ring sizes
		sysctl set num_rx_desc 1024
		sysctl set num_tx_desc 1024

		#
		# port and queues setup
		#
		port 0 mtu 1500 tpid 0x8100 state enabled
		port 1 mtu 1500 tpid 0x8100 state enabled flags qinq_enabled, dynamic_vif

		rx_queue port 0 queue 0 lcore 1
		rx_queue port 0 queue 1 lcore 2
		rx_queue port 0 queue 2 lcore 3

		rx_queue port 1 queue 0 lcore 4
		rx_queue port 1 queue 1 lcore 5
		rx_queue port 1 queue 2 lcore 6

		#
		sysctl set global_packet_counters 1
		sysctl set vif_stat 1
		sysctl set arp_cache_timeout 600
		sysctl set arp_cache_size 65536
		sysctl set dynamic_vif_ttl 600
		sysctl set fpm_debug 0
		sysctl set frag_mbuf 4000
		sysctl set mac_addr_format "linux"

		#
		# LPM DIR24-8 IPv4 FIB
		#
		sysctl set lpm_table8_size 2048	  

		# add/remove linux kernel /32 routes for ppp subscribers ip addresses.
		# Linux kernel routes are installed to 'lo' interface in the namespace therouter is running in.
		# This option allows to announce subscriber's /32 prefixes by using "redisribute kernel" command
		# in FRR/Quagga bgpd or ospfd daemons. 	  
		sysctl set install_subsc_linux_routes 1

		#
		# 3 - RTPROT_BOOT (linux netlink routes proto) 
		# Note: FRR 4.0 bgpd redistribute kernel doesn't see linux routes with proto static,
		# but it sees BOOT routes
		#
		sysctl set linux_route_proto 3

		# maximum number of VIF (vlan) interfaces
		sysctl set vif_max 10000

		# maximum number of subscriber interfaces/sessions
		sysctl set subsc_vif_max 60000
		
		# enable IPv6 protocol for IPoE subscribers
		sysctl set ipoe_ipv6 1
		
		# enable DHCPv6 server
		sysctl set dhcpv6_server 1		
	}

	runtime {
		sysctl set log_level 7

		#
		# Interfaces
		#

		# loopback
		ip addr add 4.4.4.4/32 dev lo

		# blackhole multicast addresses
		ip route add 224.0.0.0/4 unreachable

		# link with local linux host
		vif add name v20 port 0 type dot1q cvid 20
		ip addr add 192.168.20.1/24 dev v20

		# uplink
		vif add name uplink port 0 type dot1q cvid 3 flags kni,flow_acct
		ip addr add 192.168.1.112/24 dev uplink

		#
		# L2 connected subsribers
		#
		sysctl set ipoe_ttl 600
		
		#
		sysctl set install_subsc_routes 1
		
		# setup IPoE subscriber username
		ipoe subsc username format "svid.cvid.mac" delimiter "."

		#
		# Subsriber Initiation methods: 
		#   intiate subscriber sessions only by DHCP requests
		#
		sysctl set subsc_initiate_by_ingress_pkts 0
		sysctl set subsc_initiate_by_egress_pkts 0
		sysctl set l2_subsc_initiate_by_dhcp 1

		# local route to the ip address shared by all subscribers interfaces
		# according with ip unnumbered scheme
		ip route add 10.10.0.1/32 local

		# blackhole ipoe subscriber's network
		ip route add 10.10.0.0/16 unreachable  

		# default route
		ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112

		#
		# DHCP server
		#
		# enable DHCP server
		sysctl set dhcp_server 1

		# IP pools
		ip pool add pool_1
		ip pool add range pool_1 10.0.0.2 - 10.0.255.255
		ip pool set pool_1 router 10.0.0.1 mask 16 lease time 600
		ip pool add pool_1 dns 8.8.8.8
		ip pool add pool_1 dns 8.8.4.4
		ip pool add pool_1 ntp 192.36.143.130
		
		ip pool add pool_2
		ip pool add range pool_1 10.0.10.0 - 10.0.20.255
		ip pool set pool_2 router 10.0.0.1 mask 16 lease time 600
		ip pool add pool_2 dns 8.8.8.8
		ip pool add pool_2 dns 8.8.4.4
		ip pool add pool_2 ntp 192.36.143.130

		# default ipoe pools.
		# This ip pools will be used by IPoE subscribers by default 
		# if radius access-reply message doesn't specify other pool name to use
		ipoe ip pool add pool_1
		ipoe ip pool add pool_2

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
		ip route add 192.168.1.0/24 dev uplink src 192.168.1.112 table rt_bl
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
		# Flow accounting
		#
		flow ipfix_collector addr 192.168.20.3
		sysctl set flow_acct 1
		sysctl set flow_acct_dropped_pkts 0
		sysctl set flow_idle_timeout 20  

		#
		# NAT events logging (NEL)
		#
		nel collector addr 192.168.20.3 port 9995 proto 9
		sysctl set nat_events 1  

		#
		# SNAT 44
		#
		det snat create map 1 in 10.0.0.0/16 out 10.114.0.0/29 sess 4096
		det snat vif uplink enable
		det snat vif uplink add map 1  

		# blackhole NAT public (out) addresses
		ip route add 10.114.0.1/29 unreachable

		#
		# IPv6
		#
		ipv6 enable dev lo
		ipv6 addr add xxx:4/112 dev lo

		# ipv6 uplink
		ipv6 enable dev uplink
		ipv6 addr add xxx:3/112 dev uplink

		# ipv6 default route
		ipv6 route add ::/0 via xx60::1:1

		sysctl set ipv6_tcp_mss_fix 1

		# DHCPv6
		# google dns
		dhcpv6 add dns 2001:4860:4860::8888
		dhcpv6 add dns 2001:4860:4860::8844
		dhcpv6 domain search list i6.bisonrouter.com

		#
		# IPv6 Pools
		#
		# NA address pool
		ipv6 pool add na_pool xxx::/64 length 128 flags rand,cache
		# PD address pool
		ipv6 pool add pd_pool xx::/48 length 64 flags rand,cache

		# enable IA_NA, IA_PD and SLAAC for IPoE
		sysctl set ipoe_dhcpv6_ia_na 1
		sysctl set ipoe_dhcpv6_ia_pd 1
		# SLAAC is not yet supported
		sysctl set ipoe_slaac 0

		# ipv6 radius accounting. include all adresses/prefixes into start packets.
		sysctl set ppp_rad_acct_slaac 1
		sysctl set ppp_rad_acct_ia_na 1
		sysctl set ppp_rad_acct_ia_pd 1	  

		#
		# use the following ipv6 pools by default for IPoE subscriber
		# when no radius pools attributes are received
		# 
		ipoe ipv6 pool ia_na na_pool
		ipoe ipv6 pool ia_pd pd_pool
	}
