# Configuration

	startup {
	  sysctl set numa 1
	
	  # mbuf mempool size
	  # this value depends on the total number of rx queues
	  # mbuf = number of rx queues * 4096
	  sysctl set mbuf 80000
	  
	  # NIC rx/tx descriptor ring sizes
	  sysctl set num_rx_desc 1024
	  sysctl set num_tx_desc 1024
	
	  port 0 mtu 1560 tpid 0x8100 state enabled
	  port 1 mtu 1560 tpid 0x8100 state enabled flags qinq_enabled
	
	  rx_queues port 0 queues 3 lcores 1-3
	  rx_queues port 1 queues 3 lcores 1-3
		
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
	  # IP Pool serves the same purpose as DHCP lease database does
	  # and allows to have persistente ipv4 addresses as long as
	  # BisonRouter doesn't reboot (no filesystem storage)
	  #
	  sysctl set ippool_cache_size 4096
	  sysctl set ippool_cache_entry_ttl 172800
	
	  #
	  # System name
	  #
	  sysctl set system_name "br1"
	  		  
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
	  
	  #
	  # IPv6
	  #
	  
	  # LPM6
	  # note: a subscriber's /128 ipv6 address might consume up to 8 tbl8 entries
	  # when the address is allocated from /64 pool
	  #
	  sysctl set fib6_max_lpm_tbl8 262144
	  sysctl set fib6_max_next_hops 131072
	  
	  # enable ipv6 for ppp
	  sysctl set ppp_ipv6 1
	  
	  # enable DHCPv6 server
	  sysctl set dhcpv6_server 1

      sysctl set l2tp_max_lac_sessions 20000

      sysctl set pppoe_rfc_4638_mtu 1
      sysctl set pppoe_rfc_4638_mru_size_probes 1      
	}
	
	runtime {
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole NAT pool
	  ip route add 10.114.0.0/29 unreachable
	
	  #
	  # Interfaces
	  #

	  # loopback address
  	  ip addr add 5.5.5.5/32 dev lo
	  
	  # uplink interface
	  vif add name uplink port 0 type dot1q cvid 3
	  ip addr add 192.168.1.2/24 dev uplink
	  
	  # Dot1q VLAN interfaces
	  vif add name v101 port 1 type dot1q cvid 101 mtu 1560 
	  vif add name v102 port 1 type dot1q cvid 102 mtu 1560 
	  vif add name v103 port 1 type dot1q cvid 103 mtu 1560 
	  	  
	  # Management interface to a subnet with 
	  # Radius server and NetFlow collector
	  #
	  vif add name mgmt port 0 type dot1q cvid 5
	  ip addr add 192.168.5.1/24 dev mgmt

      # enable MSS fix
      sysctl set tcp_mss_fix 1
      	  
	  # 
	  # ppp
	  #
	  ppp dns primary 8.8.8.8
	  ppp dns secondary 8.8.4.4
	  	  
	  #
	  # it's an IP address that will be assigned to each ptp pppoe interface
	  #
	  ppp ipcp server ip 10.10.1.1
	  ip route add 10.10.1.1/32 local
	
	  #
	  # IP pools
	  #
	  
	  ip pool add ppp_pool_1
	  ip pool add range ppp_pool_1 10.0.1.0 - 10.0.1.255
	
	  ip pool add ppp_pool_2
	  ip pool add range ppp_pool_2 10.0.2.0 - 10.0.2.255
	  
	  # configure ppp to use ip pools 1 and 2
	  ppp ip pool add ppp_pool_1
	  ppp ip pool add ppp_pool_2
	  
	  #
	  # Radius authorization
	  #
	  
	  # specify an address assigned to the management VIF "mgmt"
	  radius_client add src ip 192.168.5.1
	  	  
	  # 
	  # use your own radius server IP address
	  #
	  radius_client add server 192.168.5.2 port 1812
	  radius_client set secret "xxx"
	  coa server set secret "xxx"
	  
	  #
	  # Radius accounting
	  #
	   
	  # replace 192.168.5.2 with the ip address of your radius server
	  #
	  radius_client add accounting server 192.168.5.2
	  radius_client set accounting secret "xx"
	
	  sysctl set radius_accounting 1
	  sysctl set radius_accounting_interim 1
	  sysctl set radius_accounting_interim_interval 600
	  sysctl set radius_initial_retransmit_timeout 500
	  	
	  #
	  # Flow accounting
	  #
	  flow collector add addr 192.168.5.3 port 9995 proto 9
	  sysctl set flow_acct 1
	  sysctl set flow_acct_dropped_pkts 0	  
	
	  #
	  # NAT events logging (NEL)
	  #
      nel collector add addr 192.168.5.4 port 9995 proto 9 events pb,nat_sess
	  sysctl set nat_events 1
	
	  #
	  # Deterministic SNAT 44
	  #
	  det snat create map 1 in 10.0.0.0/23 out 10.114.0.0/29 sess 4096
	  det snat vif uplink enable
	  det snat vif uplink add map 1
	
	  #
	  # Destination NAT44
	  #
	  det snat add dnat map 1 size 128
	  det snat add dnat rule map 1 out 10.114.0.0:23200 in 10.0.0.0:22 tcp
	
	  # default route
	  ip route add 0.0.0.0/0 via 192.168.1.1 src 192.168.1.2
	  
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
	  
	  # enable IA_NA, IA_PD and SLAAC for PPP
	  sysctl set ppp_dhcpv6_ia_na 2
	  sysctl set ppp_dhcpv6_ia_pd 2
	  sysctl set ppp_slaac 1
	  
	  # ipv6 radius accounting. Include all adresses/prefixes into start packets.
	  sysctl set ppp_rad_acct_slaac 1
	  sysctl set ppp_rad_acct_ia_na 1
	  sysctl set ppp_rad_acct_ia_pd 1	  
	  
	  #
	  # IPv6 Pools
	  #
	  
	  # SLAAC pool
	  ipv6 pool add ppp6_slaac_pool xxx::/48 length 64 flags rand,cache
	  
	  # NA address pool
	  ipv6 pool add ppp6_na_pool xxx::/64 length 128 flags rand,cache
	  
	  # PD address pool
	  ipv6 pool add ppp6_pd_pool xx::/48 length 64 flags rand,cache
	  
	  #
	  # use the following ipv6 pools by default for ppp
	  # when no radius pools attr are supplied
	  # 
	  ppp ipv6 pool ia_na ppp6_na_pool
	  ppp ipv6 pool ia_pd ppp6_pd_pool
	  ppp ipv6 pool slaac ppp6_slaac_pool

      #
      # L2TP tunnels
      #
      l2tp tunnel add name "xl2tp" peer name "ubuntu1" local name "br1" secret "xx_your_secret_xx"
      l2tp tunnel add name "v101-lns" ingress vif "v101" local name "br1"
      l2tp tunnel add name "loopback-lns" local ip 5.5.5.5 secret "s3cr3t"
	}
