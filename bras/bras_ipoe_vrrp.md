# 1. Introduction

# 3. Network scheme

<img src="http://therouter.net/images/bras/bras vrrp.png">

# 4. Configuration

## 4.1. BNG, virtual router 1

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
	  #
	  port 0 mtu 1500 tpid 0x8100 state enabled

	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3

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
	  vif add name uplink port 0 type dot1q cvid 3
	  ip addr add 192.168.1.112/24 dev uplink
	  ip addr add 10.8.0.1/24 dev uplink

	  #
	  # L2 connected subsribers
	  #

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

	  sysctl set ipoe_ttl 600

	  #
	  # Subscriber VLANs
	  #
	  vif add name v5 port 0 type dot1q cvid 5 flags l2_subs,proxy_arp

	  # qinq interfaces to L2 connected subscribers vlans
	  vif add name l2subs_a port 0 type qinq range svid 2010 cvid 2500 2800 flags l2_subs,proxy_arp

	  # dot1q interfaces to L2 connected subscribers vlans
	  vif add name l2subs_b port 0 type dot1q range svid 0 cvid 2500 2800 flags l2_subs,proxy_arp

	  #
	  # VRRP
	  #

	  # keepalive, db sync interface
	  subsc vrrp init dev uplink

	  #
	  # Subscriber VRRP group 10
	  #
	  subsc vrrp create group 10 prio 100 neighbor 10.8.0.2
	  subsc vrrp group 10 add vif port 0 cvid 5
	  subsc vrrp group 10 add vif range svid 2010 cvid 2500 2800 name l2subs_a
	  subsc vrrp group 10 add ip 192.168.2.1
	  subsc vrrp group 10 add ip 192.168.3.1
	  subsc vrrp group 10 enable

	  #
	  # Subscriber VRRP group 20
	  #
	  subsc vrrp create group 20 prio 50 neighbor 10.8.0.2
	  subsc vrrp group 20 add vif range svid 0 cvid 2500 2800 name l2subs_b
	  subsc vrrp group 20 add ip 192.168.4.1
	  subsc vrrp group 20 add ip 192.168.5.1
	  subsc vrrp group 20 enable

	  # blackhole ipoe subscriber's network
	  ip route add 192.168.0.0/16 unreachable

	  # default route
	  ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112

	  #
	  # DHCP server
	  #
	  # enable DHCP server
	  sysctl set dhcp_server 1

	  # IP pools
	  ip pool add pool_2
	  ip pool add range pool_2 192.168.2.2 - 192.168.2.255
	  ip pool set pool_2 router 192.168.2.1 mask 24 lease time 600
	  ip pool add pool_2 dns 8.8.8.8
	  ip pool add pool_2 dns 8.8.4.4
	  ip pool add pool_2 ntp 192.36.143.130

	  ip pool add pool_3
	  ip pool add range pool_3 192.168.3.2 - 192.168.3.255
	  ip pool set pool_3 router 192.168.3.1 mask 24 lease time 600
	  ip pool add pool_3 dns 8.8.8.8
	  ip pool add pool_3 dns 8.8.4.4
	  ip pool add pool_3 ntp 192.36.143.130

	  ip pool add pool_4
	  ip pool add range pool_4 192.168.4.2 - 192.168.4.255
	  ip pool set pool_4 router 192.168.4.1 mask 24 lease time 600
	  ip pool add pool_4 dns 8.8.8.8
	  ip pool add pool_4 dns 8.8.4.4
	  ip pool add pool_4 ntp 192.36.143.130

	  ip pool add pool_5
	  ip pool add range pool_5 192.168.5.2 - 192.168.5.255
	  ip pool set pool_5 router 192.168.5.1 mask 24 lease time 600
	  ip pool add pool_5 dns 8.8.8.8
	  ip pool add pool_5 dns 8.8.4.4
	  ip pool add pool_5 ntp 192.36.143.130

	  # default ipoe pools.
	  # This ip pools will be used by IPoE subscribers by default 
	  # if radius access-reply message doesn't specify other pool name to use
	  ipoe ip pool add pool_2

	  #
	  # profiles
	  #
	  ipoe profile create 1
	  ipoe profile 1 add dhcp pool pool_3
	  ipoe profile 1 add vif v5
	  ipoe profile 1 add vif range svid 2010 cvid 2500 2800 name l2subs_a

	  ipoe profile create 2
	  ipoe profile 1 add dhcp pool pool_4
	  ipoe profile 1 add vif range svid 0 cvid 2500 2800 name l2subs_b

	  #
	  # Radius
	  #
	  radius_client add src ip 192.168.20.1
	  radius_client add server 192.168.20.3
	  radius_client set secret "secret"
	  coa server set secret "secret"

	  #
	  # NAT events logging (NEL)
	  #
	  nel collector addr 192.168.20.3 port 9995 proto 9
	  sysctl set nat_events 1  

	  #
	  # SNAT 44
	  #
	  det snat create map 1 in 192.168.0.0/16 out 10.114.0.0/29 sess 4096
	  det snat vif uplink enable
	  det snat vif uplink add map 1  

	  # blackhole NAT public (out) addresses
	  ip route add 10.114.0.1/29 unreachable
	}

## 4.2. BNG, virtual router 2

	startup {

	}
	
	runtime {

	}

