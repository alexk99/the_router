# Link Bonding

Since TheRouter is a DPDK application it supports link bonding (link aggregation).
To create a link bonding port use command line parameters 
(see the section "Command line options" of <a href="conf_options.md">Configuring TheRouter</a>).

Example of a creation link bonding port:

	--vdev 'eth_bond0,mode=2,slave=0000:02:00.0,slave=0000:02:00.1,xmit_policy=l23'

Detailed description of the all link bonding parameters can be found in the DPDK Documentation
<a href="http://dpdk.org/doc/guides-16.07/prog_guide/link_bonding_poll_mode_drv_lib.html">Link Bonding Poll Mode Driver Library</a>

## Example of using link bonding ports

<img src="http://therouter.net/images/tests/link_bonding.png">

# TheRouter configaration

## router_run.sh

		#!/bin/bash
		the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' -n2 --vdev 'eth_bond0,mode=2,slave=0000:02:00.0,slave=0000:02:00.1,xmit_policy=l23' -- -c $1
	
## router.conf
	
		startup {
		  port 0 mtu 1500 tpid 0x8100 state enabled
		
		  rx_queue port 0 queue 0 lcore 1
		  rx_queue port 0 queue 1 lcore 2
		  rx_queue port 0 queue 2 lcore 3
		
		  sysctl set global_packet_counters 1
		}
		
		
		runtime {
		  vif add name v130 port 0 type dot1q cvid 130
		  ip addr add 10.0.0.1/24 dev v130
		
		  vif add name v131 port 0 type dot1q cvid 131
		  ip addr add 10.0.1.1/24 dev v131
		
		  npf load "/etc/npf.conf.accept_all"
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
			