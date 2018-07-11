# IPV6 Lab #3. Static and connected ipv6 routes.

lab#3 test network consists of border router R1 that has
ipv6 connectivity, the_router and the client machine C4. 
TheRouter connects C4 with the border router, allowing c4
to access internet via ipv6 protocol.

<img src="http://therouter.net/images/ipv6/ipv6 lab3.png">

## 1. R2 config

	startup {
		...
	}
	runtime {
	  # loopback address
	  ip addr add 5.5.5.5/32 dev lo
	
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  ipv6 enable dev v3
	  ipv6 addr link-local fe80::6a05:caff:fe30:6180 dev v3
	  ipv6 addr add 2001:470:1f0b:1460::10/64 dev v3
	
	  ipv6 enable dev v4
	  ipv6 addr add 2001:470:1f0b:1460::2:2/112 dev v4
	
	  ipv6 enable dev v100
	  ipv6 addr add 2001:470:1f0b:1460::1:2/112 dev v100
	
	  # default route
	  ipv6 route add ::/0 via 2001:470:1f0b:1460::1:1
	}

## 2. R2 routing table

	h5 ~ # rcli sh ipv6 route
	2001:470:1f0b:1460::1:0/112 C dev v100
	2001:470:1f0b:1460::2:0/112 C dev v4
	::/0 via 2001:470:1f0b:1460::1:1 dev v100

## 3. C4 config

	ip netns add c4
	export c4ns="ip netns exec c4"
	$c4ns ip link set up lo
	ip link set vlan4 netns c4
	$c4ns ip link set up vlan4
	
	$c4ns ip -6 addr add 2001:470:1f0b:1460::2:1/112 dev vlan4
	$c4ns ip -6 route add default via 2001:470:1f0b:1460::2:2

## 4. Check connectivity

	h5 ~ # rcli ping6 2001:470:1f0b:1460::1:1
	Ping 2001:470:1f0b:1460::1:1 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.858 ms
	reply 56 bytes icmp_seq=2 time=0.307 ms
	reply 56 bytes icmp_seq=3 time=0.309 ms
	reply 56 bytes icmp_seq=4 time=0.311 ms
	^C
	h5 ~ #
	h5 ~ # rcli ping6 2001:470:1f0b:1460::2:1
	Ping 2001:470:1f0b:1460::2:1 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.551 ms
	reply 56 bytes icmp_seq=2 time=0.157 ms
	reply 56 bytes icmp_seq=3 time=0.156 ms
	reply 56 bytes icmp_seq=4 time=0.159 ms
	^C
	h5 ~ #
	h5 ~ # $c4ns ping6 2001:470:1f0b:1460::2:2
	PING 2001:470:1f0b:1460::2:2(2001:470:1f0b:1460::2:2) 56 data bytes
	64 bytes from 2001:470:1f0b:1460::2:2: icmp_seq=1 ttl=64 time=0.238 ms
	64 bytes from 2001:470:1f0b:1460::2:2: icmp_seq=2 ttl=64 time=0.106 ms
	64 bytes from 2001:470:1f0b:1460::2:2: icmp_seq=3 ttl=64 time=0.105 ms
	64 bytes from 2001:470:1f0b:1460::2:2: icmp_seq=4 ttl=64 time=0.102 ms
	^C
	--- 2001:470:1f0b:1460::2:2 ping statistics ---
	4 packets transmitted, 4 received, 0% packet loss, time 3075ms
	rtt min/avg/max/mdev = 0.102/0.137/0.238/0.059 ms


### 4.1. Ping an internet host

	h5 ~ # $c4ns ping6 2a02:6b8::2:242
	PING 2a02:6b8::2:242(2a02:6b8::2:242) 56 data bytes
	64 bytes from 2a02:6b8::2:242: icmp_seq=1 ttl=54 time=66.4 ms
	64 bytes from 2a02:6b8::2:242: icmp_seq=2 ttl=54 time=65.2 ms
	64 bytes from 2a02:6b8::2:242: icmp_seq=3 ttl=54 time=65.2 ms
	64 bytes from 2a02:6b8::2:242: icmp_seq=4 ttl=54 time=65.3 ms
	^C
	--- 2a02:6b8::2:242 ping statistics ---
	4 packets transmitted, 4 received, 0% packet loss, time 3001ms
	rtt min/avg/max/mdev = 65.243/65.573/66.411/0.578 ms

		