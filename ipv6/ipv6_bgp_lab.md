# IPV6 Lab #4. BGP.

lab#4 test network consists of linux router C4 running linux and FRR,
the_router and the host C5. 

Host C5 is directly connected to router C4 and injects ipv6 full table 
into C4 using perl script bgp_simple.pl. C4 is connected to H5 TheRouter
and forward it's full ipv6 routing table to TheRouter.

<img src="http://therouter.net/images/ipv6/ipv6 lab4 bgp.png">

## 1. H5 (TheRouter) config

	startup {
	  # mbuf mempool size
	  sysctl set mbuf 8192
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  sysctl set log_level 8
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 60
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	
	  sysctl set icmp6_transmission_burst 32
	  sysctl set fib6_max_lpm_tbl8 32768
	}
	runtime {
	  vif add name v4 port 0 type dot1q cvid 4 flags kni
	  vif add name v100 port 0 type dot1q cvid 100
	
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  ipv6 enable dev lo
	  ipv6 addr add 2001:470:1f0b:1460::200:200/128 dev lo
	
	  ipv6 enable dev v4
	  ipv6 addr add 2001:470:1f0b:1460::2:2/112 dev v4
	
	  ipv6 enable dev v100
	  ipv6 addr add 2001:470:1f0b:1460::3:2/112 dev v100
	  ipv6 addr add 2001:470:1f0b:1460::1:2/112 dev v100
	
	  # default route
	  ipv6 route add ::/0 via 2001:470:1f0b:1460::1:1
	}

## 1.1. H5 (TheRouter) bgpd config

	!
	! Zebra configuration saved from vty
	!   2018/07/23 20:21:09
	!
	frr version 4.0
	frr defaults traditional
	!
	hostname h5
	password hokawuwe
	log syslog
	!
	!
	router bgp 64512
	 bgp router-id 10.1.0.1
	 coalesce-time 1000
	 timers bgp 20000 60000
	 neighbor 2001:470:1f0b:1460::2:1 remote-as 64514
	 neighbor 2001:470:1f0b:1460::2:1 update-source 2001:470:1f0b:1460::2:2
	 !
	 address-family ipv6 unicast
	  neighbor 2001:470:1f0b:1460::2:1 activate
	 exit-address-family
	!
	!
	ip prefix-list pl_norm_nets seq 5 permit 0.0.0.0/0 le 24
	!
	route-map rm_in permit 10
	 match ip address prefix-list pl_norm_nets
	!
	route-map rm_in deny 20
	!
	line vty
	!

## 2. C4 config

	ip netns add c4
	export c4ns="ip netns exec c4"
	$c4ns ip link set up lo
	ip link set vlan4 netns c4
	$c4ns ip link set up vlan4
	
	$c4ns ip -6 addr add 2001:470:1f0b:1460::2:1/112 dev vlan4
	$c4ns ip -6 route add default via 2001:470:1f0b:1460::2:2
	
## 2.1. C4 bpgd config

	!
	! Zebra configuration saved from vty
	!   2018/07/23 20:35:34
	!
	frr version 4.0
	frr defaults traditional
	!
	hostname c4
	password hokawuwe
	log syslog
	!
	!
	router bgp 64514
	 bgp router-id 10.1.0.4
	 coalesce-time 1000
	 timers bgp 20000 60000
	 neighbor 2001:470:1f0b:1460::2:2 remote-as 64512
	 neighbor 2001:470:1f0b:1460::2:2 update-source 2001:470:1f0b:1460::2:1
	 neighbor 2001:470:1f0b:1460::10:1 remote-as 64513
	 neighbor 2001:470:1f0b:1460::10:1 update-source 2001:470:1f0b:1460::10:2
	 !
	 address-family ipv6 unicast
	  neighbor 2001:470:1f0b:1460::2:2 activate
	  neighbor 2001:470:1f0b:1460::10:1 activate
	 exit-address-family
	!
	!
	ip prefix-list pl_norm_nets seq 5 permit 0.0.0.0/0 le 24
	!
	route-map rm_in permit 10
	 match ip address prefix-list pl_norm_nets
	!
	route-map rm_in deny 20
	!
	line vty
	!

## 3. C5 config

	ip link add veth0 type veth peer name veth1
	ip link set veth0 netns c5
	ip link set veth1 netns c4
	
	$c5ns ip link set up lo
	$c5ns ip link set up veth0
	$c5ns ip -6 addr add 2001:470:1f0b:1460::10:1/112 dev veth0
	
	$c4ns ip link set up lo
	$c4ns ip link set up veth1
	$c4ns ip -6 addr add 2001:470:1f0b:1460::10:2/112 dev veth1	


## 4. Inject full ipv6 table

	cd /usr/src/bgpsimple
	$c5ns ./bgp_simple.pl -myas 64513 -myip 2001:470:1f0b:1460::10:1 -peerip 2001:470:1f0b:1460::10:2 -keepalive 20000 -holdtime 60000 --peeras 64514 -p /usr/src/fulltablle/routes --nolisten -v -v -ipv6=1

## 5. Results

	h5 ~ # $rvrf rcli sh ipv6 route | wc -l
	46331
	
	h5 ~ # $rvrf rcli sh ipv6 route | head
	2605:a401:80e2::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2a00:1950::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2804:49c:3104::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2a02:26f0:b400::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2604:3e40::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2605:8000:90::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2804:14d:7e80::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2804:14d:5cd6::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2a02:b48:8102::/0 via fe80::62a4:4cff:fe41:a24 dev v4
	2a01:2b0:2000:155::/0 via fe80::62a4:4cff:fe41:a24 dev v4
		