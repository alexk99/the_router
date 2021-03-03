# IPoE IPv4 BNG example configuration.
  
In the following example 10.10.0.0/16 is a network with IPoE subscribers
and ip unnumbered scheme is used to configure IPv4 subscriber's routing.

Subscribers are connected to vlan v5, QinQ vlan range cvid 2010 cvid 2500 - 2800,
dot1q range 2500 - 2800.

To provide subscriber with IPv4 addresses DHCP relay is used wich
forward DHCP requests to an external DHCP server located at host 192.168.20.3.
host 192.168.20.3 could be running at the same machine TR is running at.

  	startup {
	  sysctl set numa 0
	
	  # mbuf mempool size
	  # this value depends on the total number of rx queues
	  # mbuf = number of rx queues * 4096
	  sysctl set mbuf 16384
	  
	  # NIC rx/tx descriptor ring sizes
	  sysctl set num_rx_desc 512
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
	  sysctl set arp_cache_timeout 300
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
	    
	  vif add name v5 port 0 type dot1q cvid 5 flags l2_subs,proxy_arp
	  ip addr add 10.10.0.1/32 dev v5
	  
	  # qinq interfaces to L2 connected subscribers vlans
	  vif add name l2subs_a port 0 type qinq range svid 2010 cvid 2500 2800 flags l2_subs,proxy_arp
	  ip addr add range svid 2010 cvid 2500 2800 10.10.0.1/32 name l2subs_a
	  
	  # dot1q interfaces to L2 connected subscribers vlans
	  vif add name l2subs_b port 0 type dot1q range svid 0 cvid 2500 2800 flags l2_subs,proxy_arp
	  ip addr add range svid 2010 cvid 2500 2800 10.10.0.1/32 name l2subs_b
	
	  # local route to the ip address shared by all subscribers interfaces
	  # according with ip unnumbered scheme
	  ip route add 10.10.0.1/32 local
	  
	  # blackhole ipoe subscriber's network
	  ip route add 10.10.0.0/16 unreachable  
	
	  # default route
	  ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112
	
	  #
	  # DHCP relay
	  #
	  dhcp_relay opt82 mode rewrite_if_doesnt_exist
	  dhcp_relay opt82 remote_id "tr_h4"
	  dhcp_relay 192.168.20.3
	  sysctl set dhcp_relay_enabled 1
	
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
	  det snat create map 1 in 10.0.0.0/16 out 10.114.0.0/29 sess 2048
	  det snat vif uplink enable
	  det snat vif uplink add map 1  
	  
	  # blackhole NAT addresses
	  ip route add 10.114.0.1/29 unreachable
	}
