# CGNAT, Carrier-grade NAT

There are three different modules implementing CGNAT functions in BisonRouter (former TheRouter) Those modules implement different sets of NAT functions.

## Port-Block based NAT

- SNAT44, DNAT44
- EIM, Endpoint Independent Mapping
- NAT session/translation limit per host
- Dynamic PBA (port-block allocation)
- NEL: NAT events logging: IPFIX, Netflow v9
- Performance: up to 15Mpps, up to 120Gbit/s
- ALG support: PPtP
- <a href="https://docs.bisonrouter.com/runtime_commands.html#snat44">NAT Docs</a>

## Deterministic NAT

- SNAT44, DNAT44
- EIM, Endpoint Independent Mapping
- NAT session/translation limit per host
- Deterministic address and port selection
- NEL: NAT events logging: IPFIX, Netflow v9
- Performance: up to 15Mpps, up to 120Gbit/s
- ALG support: PPtP
- <a href="https://docs.bisonrouter.com/runtime_commands.html#deterministic-snat44">Deterministic NAT Docs</a>

## NPF

- SNAT44, DNAT44 modes
- Endpoint (Address and Port) Dependent Mapping
- NAT session/translation limit per host
- NAT events logging: IPFIX, netflow v9
- Performance: up to 8Mpps, up to 60Gbit/s
- ALG support: PPtP


## Configuration example

### Deterministic SNAT44

	startup {
	  sysctl set num_rx_desc 1024
	  sysctl set num_tx_desc 1024
	
	  # mbuf mempool size
	  sysctl set mbuf 32768
	
	  # mode 4 - lacp
	  bond port mode 4 bond_slaves 0,1 primary port 1 policy 2
	  port 2 mtu 1500 tpid 0x8100 state enabled bond_slaves 0,1
	
	  rx_queue port 2 queue 0 lcore 1
	  rx_queue port 2 queue 1 lcore 2
	  rx_queue port 2 queue 2 lcore 3
	
	  sysctl set log_level 8
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 600
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 300
	  sysctl set vif_stat 1
	  sysctl set frag_mbuf 4000
	  sysctl set mac_addr_format "linux"
	  sysctl set nd_neighbor_cache_entry_ttl 120
	
	
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
	
	  #
	  sysctl set nat_portmap_initial_size 16777216
	}
	
	runtime {
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  # blackhole NAT addresses
	  # ip route add 10.114.0.0/29 unreachable
	
	  vif add name uplink port 2 type dot1q cvid 3
	  vif add name v20 port 2 type dot1q cvid 20
	  
	  ip addr add 192.168.1.111/24 dev uplink
	  ip addr add 192.168.20.1/24 dev v20
	
	  #
	  # NAT events (NEL)
	  #
	  nel collector addr 192.168.20.2 port 9995 proto 9
	  sysctl set nat_events 1
	
	  #
	  # Deterministic NAT
	  #
	  det snat create map 1 in 10.11.1.0/24 out 10.114.0.0/29 sess 2048
	  det snat vif uplink enable
	  det snat vif uplink add map 1	
	}


## Production cases

### PPPoE BNG + CGNAT

A BNG server running 20k+ PPPoE subscribers and performing CGNAT

<img src="http://therouter.net/images/production/det_nat1/det_nat_br3_2.png">
<img src="http://therouter.net/images/production/det_nat1/det_nat_br3_2_pps.png">

