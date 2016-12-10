# Source NAT

NAT functionality of the router is based on the NPF standalone lib. So the configuration process here 
is very similar to the configuration of the NetBSD packet filter except that 
a fork of NPF lib is used which has a lot of new code and uses different core for handling
the connection state database.

To describe and measure the perfomance of source nat function of the router a testing lab was build.
The lab includes 3 servers and one L2 switch that connects all the servers into L2 network devided into
2 vlans. From L3 point of view the lab consists of two hosts and a router between them. Host S1 establish
tcp connections to the host S3 and the router perform source NAT for those tcp connections. To established
big number of connections the <a href="https://github.com/Juniper/warp17">Stateful Traffic Generator WARP17</a> 
is used on both hosts S1 and S2.


## Testing lab

<img src="therouter.net/images/tests/source_nat/nat_warp_17_lab.png">

## Hardware configuration

### Server - S1
	- Processor: Intel(R) Core(TM) i5-2400 CPU @ 3.10GHz
	- NIC: Intel X520-DA2
	- Ram: 8Gb DDR3
	
### Server - S2
	- Processor: Intel(R) Core(TM) i5-3470 CPU @ 3.20GHz
	- NIC: Intel X520-DA2
	- Ram: 8Gb DDR3
	
### Server - S3
	- Processor: Intel(R) Core(TM) i7-5820K CPU @ 3.30GHz
	- NIC: Intel X520-DA2
	- Ram: todo
	- Motheboard: todo

## S2 Router configuration

	- /etc/router.conf

	```
	startup {
	  port 0 mtu 1500 tpid 0x8100 state enabled
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	  rx_queue port 0 queue 3 lcore 4
	  rx_queue port 0 queue 4 lcore 5
	
	  rx_queue port 1 queue 0 lcore 5
	  rx_queue port 1 queue 1 lcore 4
	  rx_queue port 1 queue 2 lcore 3
	  rx_queue port 1 queue 3 lcore 2
	  rx_queue port 1 queue 4 lcore 1
	
	  sysctl set global_packet_counters 1
	}
	
	runtime {
	  vif add name p0 port 1 type untagged
	  ip addr add 10.0.0.1/24 dev p0
	
	  vif add name p1 port 0 type untagged
	  ip addr add 10.0.1.1/24 dev p1
	
	  ip route add 0.0.0.0/0 via 10.0.1.2 src 10.0.1.1
	
	  npf load "/etc/npf.conf.warp17"
	}

	```
	
	- /etc/npf.conf.warp17
	
	```
	map p1 netmap 10.0.100.0/24
	
	group default {
	  pass stateful final on p1 all
	  pass final on p0 all
	}
	```

## S1 Warp17 - client
	```
	set tests mtu port 0 1500
	add tests l3_intf port 0 ip 10.0.0.2 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.3 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.4 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.5 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.6 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.7 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.8 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.9 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.10 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.11 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.12 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.13 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.14 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.15 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.16 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.17 mask 255.255.255.0
	add tests l3_gw port 0 gw 10.0.0.1
	
	add tests client tcp port 0 test-case-id 0 src 10.0.0.2 10.0.0.17 sport 1001 60000 dest 10.0.1.2 10.0.1.2 dport 6001 6001
	set tests timeouts port 0 test-case-id 0 init 4
	set tests timeouts port 0 test-case-id 0 uptime infinite
	set tests timeouts port 0 test-case-id 0 downtime 2
	set tests rate port 0 test-case-id 0 open infinite
	set tests rate port 0 test-case-id 0 send infinite
	set tests criteria port 0 test-case-id 0 run-time 600
	set tests client raw port 0 test-case-id 0 data-req-plen 64 data-resp-plen 64
	
	start tests port 0
	show tests ui
	```

## S3 Warp17 - server

	```
	add tests l3_intf port 0 ip 10.0.1.2 mask 255.255.255.0
	add tests l3_gw port 0 gw 10.0.1.1
	add tests server tcp port 0 test-case-id 0 src 10.0.1.2 10.0.1.2 sport 6001 6001
	set tests server raw port 0 test-case-id 0 data-req-plen 64 data-resp-plen 64
	start tests port 0
	show tests ui
	```

## Results

To measure the final results L2 switch port counters were used.

### Test in progress...

```
 Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              15838404480                   380334448
 RX Frames   	       117083587                     2795888
 TX Bytes     	       15964455088                   380334006
 TX Frames   	       117699235                     2795884
```

### Test is completed

``` 
 Port Number : 26 
  Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              225496441008                  0
 RX Frames   	       1658751795                    0
 TX Bytes     	       225633678544                  0
 TX Frames   	       1658908594                    0
 
 Port Number : 27
 Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              225585011456                  0
 RX Frames   	       1658658581                    0
 TX Bytes     	       225525397824                  0
 TX Frames   	       1659007125                    0
```