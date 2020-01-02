# 1. Introduction

# 3. Network scheme

<img src="http://therouter.net/images/bras/bras vrrp.png">

# 4. Configuration

## 4.1. BNG, virtual router 1

	startup {
	  # mbuf mempool size
	  sysctl set mbuf 16384
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  sysctl set log_level 8
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 600
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	  sysctl set frag_mbuf 4000
	  sysctl set mac_addr_format "linux"
	
	  sysctl set flow_acct 1
	  sysctl set flow_acct_dropped_pkts 1
	
	  sysctl set radius_max_sessions 20000
	  sysctl set subsc_vif_max 50000
	  sysctl set system_name "tr1"
	
	  sysctl set radius_accounting 0
	  sysctl set radius_accounting_interim 1
	  sysctl set radius_accounting_interim_interval 600
	  sysctl set radius_initial_retransmit_timeout 500
	
	  # any protocol timeouts (UDP)
	  sysctl set NPF_ANY_CONN_CLOSED 2
	  sysctl set NPF_ANY_CONN_NEW 30
	  sysctl set NPF_ANY_CONN_ESTABLISHED 60
	
	  # TCP timeouts
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
	
	  sysctl set lpm_table8_size 2048
	
	  sysctl set l2_subsc_initiate_by_dhcp 1
	  sysctl set l2_subsc_arp_security 1
	
	  sysctl set subsc_initiate_by_egress_pkts 0
	  sysctl set subsc_initiate_by_ingress_pkts 0
	
	  sysctl set subsc_update_expiration_by_ingress_pkts 0
	  sysctl set subsc_update_expiration_by_egress_pkts 0
	
	}
	
	runtime {
	  flow ipfix_collector addr 192.168.1.73
	
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole NAT addresses
	  # ip route add 10.114.0.0/29 unreachable
	
	  #
	  # Interfaces
	  #
	  vif add name v3 port 0 type dot1q cvid 3 flags pppoe_on,npf_on,kni,flow_acct
	  vif add name v20 port 0 type dot1q cvid 20
	
	  arp add 192.168.1.73 60:a4:4c:41:0a:24 dev v3 static
	  arp add 192.168.20.2 60:a4:4c:41:0a:24 dev v20 static
	
	  ip addr add 192.168.1.111/24 dev v3
	  ip addr add 192.168.1.118/24 dev v3
	
	  ip addr add 192.168.20.111/24 dev v20
	  ip addr add 192.168.20.112/24 dev v20
	
	  #
	  # radius
	  #
	  radius_client add src ip 192.168.20.111
	  radius_client add src ip 192.168.20.112
	
	  radius_client add server 192.168.20.3 port 1812
	  radius_client set secret "xx"
	  coa server set secret "xx"
	
	  radius_client add accounting server 192.168.20.2
	  radius_client set accounting secret "xx"
	
	  #
	  # DHCP relay
	  #
	  # dhcp_relay opt82 mode rewrite_off
	  #
	  sysctl set dhcp_relay_enabled 1
	  dhcp_relay opt82 mode rewrite_if_doesnt_exist
	  dhcp_relay opt82 remote_id "tr_h5"
	  dhcp_relay 192.168.20.3
	
	  #
	  # Subscriber VRRP
	  #
	  subsc vrrp init dev v3
	  sysctl set subsc_vrrp_master_down_timeout 10000
	  sysctl set log_level 7
	
	  #
	  # VRRP group 1
	  #
	  vif add name v5 port 0 type dot1q cvid 5 flags l2_subs,proxy_arp
	  ip addr add 192.168.5.5/32 dev v5
	  ip route add 192.168.5.5/32 local
	
	  subsc vrrp create group 1 prio 5 neighbor 192.168.1.112
	  subsc vrrp group 1 primary ip 192.168.5.10
	  subsc vrrp group 1 add vif port 0 svid 0 cvid 5
	  subsc vrrp group 1 enable
	
	  #
	  # VRRP group 2
	  #
	
	  # vlan6
	  vif add name v6 port 0 type dot1q cvid 6 flags l2_subs,proxy_arp
	  ip addr add 192.168.6.5/32 dev v6
	
	  # vlan 8-15
	  vif add name gr2_sub port 0 type dot1q range svid 0 cvid 8 15 flags l2_subs,proxy_arp
	  ip addr add range svid 0 cvid 8 15 192.168.6.5/32 name gr2_sub
	
	  # local route to group unnumbered ip address
	  ip route add 192.168.6.5/32 local
	
	  subsc vrrp create group 2 prio 10 neighbor 192.168.1.112
	  subsc vrrp group 2 primary ip 192.168.6.10
	  subsc vrrp group 2 add vif port 0 svid 0 cvid 6
	  subsc vrrp group 2 add vif range svid 0 cvid 8 15 name gr2_sub
	  subsc vrrp group 2 enable
	
	  #
	  # NAT
	  #
	  npf load "/etc/npf.conf.bras_dhcp_relay_npf_alg"
	}

## 4.2. BNG, virtual router 2

	startup {
	  sysctl set mbuf 16384
	  sysctl set log_level 7
	  sysctl set numa 1
	
	  #
	  # port and queues setup
	  #
	  port 0 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
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
	  sysctl set fpm_debug 0
	  sysctl set lpm_table8_size 2048
	  sysctl set install_subsc_linux_routes 1
	
	  sysctl set l2_subsc_initiate_by_dhcp 1
	  sysctl set l2_subsc_arp_security 1
	
	  sysctl set subsc_initiate_by_egress_pkts 0
	  sysctl set subsc_initiate_by_ingress_pkts 0
	
	  sysctl set subsc_update_expiration_by_ingress_pkts 0
	  sysctl set subsc_update_expiration_by_egress_pkts 0
	
	  sysctl set radius_max_sessions 20000
	}
	
	runtime {
	  #
	  # NPF timeouts
	  #
	
	  # any protocol timeouts (UDP)
	  sysctl set NPF_ANY_CONN_CLOSED 0
	  sysctl set NPF_ANY_CONN_NEW 30
	  sysctl set NPF_ANY_CONN_ESTABLISHED 60
	
	  sysctl set pppoe_max_online_subsc 10
	
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
	  # Flow accounting
	  #
	  flow ipfix_collector addr 192.168.1.165
	
	  #
	  # Static routes
	  #
	
	  # blackhole nat address
	  ip route add 10.111.0.0/29 unreachable
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole ipoe subscriber's prefix
	  ip route add 10.10.0.0/24 unreachable
	
	  #
	  # Interfaces
	  #
	
	  # loopback
	  ip addr add 4.4.4.4/32 dev lo
	
	  # link with local linux host
	  vif add name v20 port 0 type dot1q cvid 20 flags rpf
	  ip addr add 192.168.20.1/24 dev v20
	
	  # uplink
	  vif add name v3 port 0 type dot1q cvid 3 flags npf_on, kni, flow_acct
	  ip addr add 192.168.1.112/24 dev v3
	
	  #
	  # DHCP relay
	  #
	  # dhcp_relay opt82 mode rewrite_off
	  dhcp_relay opt82 mode rewrite_if_doesnt_exist
	  dhcp_relay opt82 remote_id "tr_h4"
	  dhcp_relay 192.168.20.3
	
	  #
	  # Radius
	  #
	  radius_client add src ip 192.168.20.1
	  radius_client add server 192.168.20.3
	  radius_client set secret "xx"
	  coa server set secret "xx"
	
	  #
	  # radius accounting
	  #
	  sysctl set radius_accounting 1
	  sysctl set radius_accounting_interim 1
	  sysctl set radius_accounting_interim_interval 600
	  radius_client add accounting server 192.168.20.3
	  radius_client set accounting secret "xxx"
	
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
	  # Subscriber VRRP
	  #
	  subsc vrrp init dev v3
	  sysctl set subsc_vrrp_master_down_timeout 10000
	
	  # 
	  # VRRP group 1
	  #
	  vif add name v5 port 0 type dot1q cvid 5 flags kni,l2_subs,proxy_arp
	  
	  subsc vrrp create group 1 prio 10 neighbor 192.168.1.111
	  subsc vrrp group 1 add vif port 0 svid 0 cvid 5
	  subsc vrrp group 1 primary ip 192.168.5.10
	  subsc vrrp group 1 enable
	
	  #
	  # VRRP group 2
	  #
	  # vlan 6
	  vif add name v6 port 0 type dot1q cvid 6 flags kni,l2_subs,proxy_arp
	  # vlan 8-15
	  vif add name gr2_sub port 0 type dot1q range svid 0 cvid 8 15 flags l2_subs,proxy_arp
	  
	  subsc vrrp create group 2 prio 5 neighbor 192.168.1.111
	  subsc vrrp group 2 primary ip 192.168.6.10
	  subsc vrrp group 2 add vif port 0 svid 0 cvid 6
	  subsc vrrp group 2 enable
	
	  #
	  # NAT events
	  #
	  sysctl set ipfix_nat_events 1
	  ipfix_collector addr 192.168.20.3
	
	  #
	  # NPF (NAT)
	  #
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}

