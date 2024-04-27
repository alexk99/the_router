# FRR VRF test lab

<img src="http://therouter.net/images/frr_vrf_test_lab.jpg">

## Setup linux VRF

	export brns="ip netns exec br"

	# create VRF inside the_router namespace
	$brns ip link add green type vrf table 11
	$brns ip link add red type vrf table 10

	# up vrf devices
	$brns ip link set up green
	$brns ip link set up red

## Start TheRouter

### Router's conf

	startup {
	  # mbuf mempool size
	  sysctl set mbuf 8192
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	}
	
	runtime {
          sysctl set fpm_rt_map 1
	  
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  vif add name v11 port 0 type dot1q cvid 11 flags kni
	  ip addr add 10.1.0.1/24 dev v11
	
	  vif add name v12 port 0 type dot1q cvid 12 flags kni
	  ip addr add 10.2.0.1/24 dev v12
	
	  vif add name v13 port 0 type dot1q cvid 13 flags kni
	  ip addr add 10.3.0.1/24 dev v13
	
	  vif add name v14 port 0 type dot1q cvid 14 flags kni
	  ip addr add 10.4.0.1/24 dev v14
	
	  # create additional route tables
	  ip route table add green
	  ip route table add red
	
	  # create mappings between linux VRF names and router's additional route table names
	  fpm route table map add green rtable green
	  fpm route table map add red rtable red
	
	  # create sets containing VIF identificators
	  u32set create l2set_green size 4096 bucket_size 16
	  u32set create l2set_red size 4096 bucket_size 16
	
	  # create PBR rules
	  ip pbr rule add prio 10 u32set l2set_green type "l2" table green
	  ip pbr rule add prio 20 u32set l2set_red type "l2" table red
	
	  # fill l2 sets
	  l2set add l2set_green port 0 svid 0 cvid 11
	  l2set add l2set_green port 0 svid 0 cvid 13
	
	  l2set add l2set_red port 0 svid 0 cvid 12
	  l2set add l2set_red port 0 svid 0 cvid 14
	}


## Router post start script

### Note: change MAC addresses

	# Enslave L3 interfaces to a VRF device
	$rns ip link set up address 68:05:CA:30:61:80 master green dev rkni_v11
	$rns ip link set up address 68:05:CA:30:61:80 master green dev rkni_v13
	$rns ip link set up address 68:05:CA:30:61:80 master red dev rkni_v12
	$rns ip link set up address 68:05:CA:30:61:80 master red dev rkni_v14

## FRR bgpd conf

	!
	! Zebra configuration saved from vty
	!   2018/03/14 11:45:56
	!
	frr version 4.0
	frr defaults traditional
	!
	hostname tr
	log syslog
	!
	!
	router bgp 64512 vrf green
	 bgp router-id 10.1.0.1
	 coalesce-time 1000
	 neighbor 10.1.0.2 remote-as 64513
	 neighbor 10.1.0.2 update-source 10.1.0.1
	 neighbor 10.3.0.2 remote-as 64514
	 neighbor 10.3.0.2 update-source 10.3.0.1
	!
	
	router bgp 64512 vrf red
	 bgp router-id 10.2.0.1
	 coalesce-time 1000
	 neighbor 10.2.0.2 remote-as 64515
	 neighbor 10.2.0.2 update-source 10.2.0.1
	 neighbor 10.4.0.2 remote-as 64516
	 neighbor 10.4.0.2 update-source 10.4.0.1
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

## FRR zebra conf

	hostname h5
	
	vrf green
	vrf red
	!
	
	! Turn off welcome messages
	no banner motd
	
	log file /var/log/frr/zebra.log
	
	ip route 10.101.0.0/24 Null0 vrf green
	ip route 10.102.0.0/24 Null0 vrf red

## C1 bgpd conf

	!
	! Zebra configuration saved from vty
	!   2018/03/14 13:36:34
	!
	frr version 4.0
	frr defaults traditional
	!
	hostname c1
	log syslog
	!
	!
	router bgp 64513
	 bgp router-id 10.1.0.2
	 coalesce-time 1000
	 neighbor 10.1.0.1 remote-as 64512
	 !
	 address-family ipv4 unicast
	  network 10.1.1.0/24
	 exit-address-family
	!
	!
	line vty
	!

## C1 zebra conf

	hostname c1
	!
	
	! Turn off welcome messages
	no banner motd
	
	log file /var/log/frr/zebra_c1.log

	
