## Bonding port configuration

<img src="http://therouter.net/images/tests/link_bonding.png">

# TheRouter configaration
	
	startup {
	  sysctl set mbuf 8192

	  bond port mode 4 bond_slaves 0,1 primary port 1 policy 2
	  port 2 mtu 1500 tpid 0x8100 state enabled bond_slaves 0,1

	  rx_queue port 2 queue 0 lcore 1
	  rx_queue port 2 queue 1 lcore 2
	  rx_queue port 2 queue 2 lcore 3

	  sysctl set global_packet_counters 1
	}

	runtime {
	  vif add name v130 port 2 type dot1q cvid 130
	  ip addr add 10.0.0.1/24 dev v130

	  vif add name v131 port 2 type dot1q cvid 131
	  ip addr add 10.0.1.1/24 dev v131
	}

## dlink configaration

	create link_aggregation group_id 1 type static
	config link_aggregation group_id 1 master_port 13 ports 13,15 state enable
	config link_aggregation algorithm ip_source_dest
	
# Testing

	- On S3 run two iperf servers:
		
			iperf -B 10.0.0.2 -s
		
			iperf -B 10.0.0.3 -s	
		
	- On S1 run two iperf clients at the same time:
	
			iperf -t 20 -c 10.0.0.3
			------------------------------------------------------------
			Client connecting to 10.0.0.3, TCP port 5001
			TCP window size: 85.0 KByte (default)
			------------------------------------------------------------
			[  3] local 10.0.1.2 port 47406 connected with 10.0.0.3 port 5001
			[ ID] Interval       Transfer     Bandwidth
			[  3]  0.0-20.0 sec  2.17 GBytes   930 Mbits/sec
			
			
			iperf -t 20 -c 10.0.0.2
			------------------------------------------------------------
			Client connecting to 10.0.0.2, TCP port 5001
			TCP window size: 85.0 KByte (default)
			------------------------------------------------------------
			[  3] local 10.0.1.2 port 46494 connected with 10.0.0.2 port 5001
			[ ID] Interval       Transfer     Bandwidth
			[  3]  0.0-20.0 sec  2.17 GBytes   931 Mbits/sec
			
