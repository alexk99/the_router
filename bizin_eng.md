# First experience using TheRouter as core router in a service provider network

A month ago, 15 may 2017 a decision was made to start the first production 	
operation of TheRouter software in a real network environment of a service provider
network with more than 60 thousands of users.

A new router running TheRouter software had to accomplish the following goals:

- forwarding traffic between the BRAS cluster and the Google cache server;
- forwarding traffic between the Google cache server and Internet via the border router;
- making forwarding decisions based on BGP routing rules, 
BGP peering with BRAS servers and the border router;
- receipt a full bgp view/table from the border router;

The whole experiment, the goals and a the_router place in the network were chosen  
keeping in mind that the_router is still a very experimental software and it has not been used
in a real network environment before. The most important condition was to be able
to stop/restart the_router without an impact to the network operations.

# TheRouter server

 - HP ProLiant DL380 G6, 2 Intel cpu 6C X5650 2.66 GHz, 48GB DRAM
 - NICs: Intel XL710
 - Note: only single cpu is used in the the_router configuration, in order to avoid "slow" NUMA paths.

# TheRouter configuration file:

* /etc/router.conf

		startup {
		  # mbuf mempool size
		  sysctl set mbuf 16384
		
		  port 0 mtu 1500 tpid 0x8100 state enabled
		  port 1 mtu 1500 tpid 0x8100 state enabled
		  port 2 mtu 1500 tpid 0x8100 state enabled
		  port 3 mtu 1500 tpid 0x8100 state enabled
		
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
		
		  rx_queue port 2 queue 0 lcore 1
		  rx_queue port 2 queue 1 lcore 2
		  rx_queue port 2 queue 2 lcore 3
		  rx_queue port 2 queue 3 lcore 4
		  rx_queue port 2 queue 4 lcore 5
		
		  rx_queue port 3 queue 0 lcore 5
		  rx_queue port 3 queue 1 lcore 4
		  rx_queue port 3 queue 2 lcore 3
		  rx_queue port 3 queue 3 lcore 2
		  rx_queue port 3 queue 4 lcore 1
		
		  sysctl set global_packet_counters 1
		}
		
		
		runtime {
		  # blackhole multicast addresses
		  ip route add 224.0.0.0/4 unreachable
		
		  # nas clients
		  ip route add xxx.xxx.0/21 unreachable
		  ip route add xxx.xxx.0/22 unreachable
		  ip route add xxx.xxx.0/21 unreachable
		  ip route add xxx.xxx.0/22 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/21 unreachable
		
		  vif add name p0 port 0 type untagged flags kni
		  ip addr add xxx.xxx.92/28 dev p0
		
		  vif add name p2 port 2 type untagged flags kni
		  ip addr add xxx.xxx.97/27 dev p2
		
		  # default to mx
		  ip route add 0.0.0.0/0 via xxx.xxx.81 src xxx.xxx.92
		}


### Network scheme
<img src="http://therouter.net/images/production/bzn/bizin.png">

## Results and stats:

### Number of installed routes in TheRouter FIB:

		the_router ~ # rcli sh ip route | wc -l
		662055

### Last 7 days
<img src="http://therouter.net/images/production/bzn/traffic_7days.png">

### Last day
<img src="http://therouter.net/images/production/bzn/traffic_last_day.png">

## LAG versions of the experiment

Two 10G ports of the router were added to a LAG group.

### 1 day stat
<img src="http://therouter.net/images/production/bzn/the_router_bizin_monitor_lag_1.png">

### 60 days stat
<img src="http://therouter.net/images/production/bzn/the_router_bizin_monitor_lag_2.png">
