# Configuring TheRouter

There are three groups of configuration options and commands:

 * command line options
 * configuration file commands
 * rcli commands
	
## Command line options
Command line options can be modified by editing the run script /usr/local/sbin/router_run.sh.
Most of them are DPDK EAL command line options, therefore you can
refer to DPDK documentation <a href="http://dpdk.org/doc/guides/testpmd_app_ug/run_app.html?highlight=eal%20options">EAL Command-line Options</a>
for detailed description of them.

Before running TheRouter you must check the following options and use your own values depending on the hardware you use:
 * -c
 
Set the hexadecimal bitmask of the cores to run on.
	
 * --lcores
 
 !  Note: the_router.a0.01.tar.gz version has supported only a 4 cores cpu. You must use at least a 4 cores cpu and then configure
 TheRouter to use 4 lcores using --lcores='0@0,1@1,2@2,3@3' parameter
 
 
Map lcore set to physical cpu set

 * -n
 
Set the number of memory channels to use.

 * -w
 
Add a PCI device in white list.

Example of the startup script cmd options:

	the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -c $1 -d

Note:
Lcore 0 will be used for TheRouter's control plane task and can be shared with any linux tasks.
The other cores will be used in TheRouter's data plane process. You should isolate them during the linux starup process by using
linux kernel command line parameters isolcpus. Otherwise, performance of TheRouter's working threads
could be very low due the context switching.

## Configuration file options

This options are stored in the /etc/router.conf file.

Configuration file consists of the sections:
 * startup
 * runtime

Each section contains commands. Everything on the same line is considered as a single command.
Symbol # is used to comment a whole line.

	startup {
		startup_command_1
		startup_command_2
		...
		startup_command_n
	}
	
	
	runtime {
		runtime_command_1
		runtime_command_2
		...
		runtime_command_n
	}

### Configuration file example

	startup {
	  # total number of mbufs
	  sysctl set mbuf 8192
	
	  port 0 mtu 1500 tpid 0x8100 state enabled
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  rx_queue port 1 queue 0 lcore 3
	  rx_queue port 1 queue 1 lcore 2
	  rx_queue port 1 queue 2 lcore 1
	
	  sysctl set global_packet_counters 1
	
	#  sysctl set arp_cache_timeout 300
	}

	runtime {
	  vif add name p0 port 1 type untagged
	  ip addr add 10.0.0.1/24 dev p0
	
	  vif add name p1 port 0 type untagged
	  ip addr add 10.0.1.1/24 dev p1
	
	  ip route add 0.0.0.0/0 via 10.0.1.2 src 10.0.1.1
	
	  npf load "/etc/npf.conf"
	}

### NPF configaration file example

	group default {
	  pass final on p0 all
	  pass final on p1 all
	}

## Startup commands

Startup commands are the commands that are used to initilize TheRouter's susbystem and properties 
that can't be modified once TheRouter have started. This commands can only be used in
the startup section of the configuration file and can't be used by the rcli interface.

 * port

		port <dpdk_port_number> mtu <mtu_size> tpid <tpid_value> state enabled
 
 * rx_queue
 
	 	rx_queue port <dpdk_port_number> queue <queue_number> lcore <lcore>
 	
 * sysctl

		sysctl set <name> <value>
 
 * npf load

	 	npf load "<path_to_npf_configuration_file>"

	 
 Note: that you should enclose path to file with '"'.

## Runtime commands 
Runtime commands are the commands that can be either executed via rcli interface or used in the runtime section of
the configuration file.

### ip addr
 * ip addr add

		rcli ip addr add <net>/<mask> dev <vif_name>
	
 * ip addr del

		rcli ip addr del <net>/<mask> dev <vif_name>

 * sh ip addr  

		rcli sh ip addr

### ip route tables
 * ip route table add

		rcli ip route table add <route_table_name>
		
 * ip route table del

		rcli ip route table del <route_table_name>
	
 * rcli sh ip route tables

		rcli rcli sh ip route tables

### U32 sets
 * u32set create

		rcli u32set create <u32set_name> size <size> bucket_size <bucket_size>
		
 * u32set destroy

		rcli u32set destroy <u32set_name>

 * ipset add

		rcli ipset add <u32set_name> <ipv4>
 
 * ipset del

		rcli ipset del <u32set_name> <ipv4>
 
 * ipset test

		rcli ipset test <u32set_name> <ipv4>
		
 * l2set add

		rcli l2set add <u32set_name> port <port_number> svid <svid> cvid <cvid>

 * l2set del

		rcli l2set del <u32set_name> port <port_number> svid <svid> cvid <cvid>
		
 * l2set test

		rcli l2set test <u32set_name> port <port_number> svid <svid> cvid <cvid>
		

### PBR rules
 * sh ip pbr rules

		rcli sh ip pbr rules
		
 * ip pbr rule add

		ip pbr rule add prio <prio_num> u32set <u32set_name> type "ip" table <route_table_name>

		ip pbr rule add prio <prio_num> u32set <u32set_name> type "l2" table <route_table_name>
		
		rcli ip pbr rule add prio <prio_num> from <net/mask> <route_table_name>
		
 * ip pbr rule del

		rcli ip pbr rule del prio <prio_num>

 * ip pbr flush

		rcli ip pbr flush

	
### ip route
 * ip route add

		rcli ip route add <net>/<mask> dev <vif_name> src <src_ip> [table <table_name>]
		
	or
	
		ip route add <net>/<mask> via <gw_ip> src <src_ip> [table <table_name>]
		
	or
	
		ip route add <net>/<mask> unreachable [table <table_name>]
   
 * ip route del
 
		ip route del <net/mask> [table <table_name>]
   
 * sh ip route
 
		rcli sh ip route
   
### vif
 * vif add

		rcli vif add name <name> port <port_num> type <type> [svid <svid>] [cvid <cvid>] [flags <flag1,flag2...>]

   Type parameter can take one of the following values:
	 - untagged
     - dot1q
     - qinq
   
   Flags:
	 - kni
	 - proxy_arp
   	   
 * vif del

		rcli vif del <name>

 * sh vif

		rcli sh vif
   
 * sh vif counters

		rcli sh vif counters
   
 * clear vif counters

		rcli clear vif counters

### arp
   
 * arp add

		rcli arp add <ip> <mac> dev <vif_name> [static]

 * arp del
 
		rcli arp del <ip> dev <vif_name>
    
 * sh arp cache
 
		rcli sh arp cache

### sysctl

 * sysctl set

		rcli sysctl set <name> <value>

 * sysctl set

		rcli sysctl get <name> <value>

### ping

 * ping

		rcli ping --help
		Usage: ping [-c,--count count] [-i,--interval interval_in_ms] [-s icmp_payload_size]
		[-f,--dont_frag] [-a,--source_address ip_source_address] [-w,--nowait]
		[-h,--help] destination

### NPF
 * sh npf conndb size

		rcli sh npf conndb size
   
 * sh npf conndb summary

		rcli sh npf conndb summary

 * sh npf stat

		rcli sh npf stat

 * npf clear stat

		rcli npf clear stat

#### NPF sysctl variables controlling connection tracking state timeouts

	* NPF_TCPS_CLOSED
	* NPF_TCPS_SYN_SENT
	* NPF_TCPS_SIMSYN_SENT
	* NPF_TCPS_SYN_RECEIVED
	* NPF_TCPS_ESTABLISHED
	* NPF_TCPS_FIN_SENT
	* NPF_TCPS_FIN_RECEIVED
	* NPF_TCPS_CLOSE_WAIT
	* NPF_TCPS_FIN_WAIT
	* NPF_TCPS_CLOSING
	* NPF_TCPS_LAST_ACK
	* NPF_TCPS_TIME_WAIT
	* NPF_ANY_CONN_CLOSED
	* NPF_ANY_CONN_NEW
	* NPF_ANY_CONN_ESTABLISHED

Example:
	
	startup {
		...
	
		# any protocol timeouts (UDP)
		sysctl set NPF_ANY_CONN_CLOSED 2
		sysctl set NPF_ANY_CONN_NEW 30
		sysctl set NPF_ANY_CONN_ESTABLISHED 60
		
		# TCP timeouts
		sysctl set NPF_TCPS_CLOSED 10
		sysctl set NPF_TCPS_SYN_SENT 30
		sysctl set NPF_TCPS_SIMSYN_SENT 30
		sysctl set NPF_TCPS_SYN_RECEIVED 60
		sysctl set NPF_TCPS_ESTABLISHED 600
		sysctl set NPF_TCPS_FIN_SENT 240
		sysctl set NPF_TCPS_FIN_RECEIVED 240
		sysctl set NPF_TCPS_CLOSE_WAIT 45
		sysctl set NPF_TCPS_FIN_WAIT 60
		sysctl set NPF_TCPS_CLOSING 30
		sysctl set NPF_TCPS_LAST_ACK 30
		sysctl set NPF_TCPS_TIME_WAIT 120
	
		...
	}

### NAT events logging via IPFIX

 * Enable nat events 

		sysctl set ipfix_nat_events 1


 * Setup ipfix collector

		ipfix_collector addr 192.168.20.2

### Other commands

 * shutdown

		rcli shutdown

### Router statistic commands

 * sh port ext stat

		rcli sh port ext stat

 * sh port stat
 
		rcli sh port stat

 * sh cmbuf stats

		rcli sh cmbuf stats

 * sh mbuf stats

		rcli sh mbuf stats

 * sh stats

		rcli sh stats
   
 * clear stats
 
		rcli clear stats

## IPv6

 * ipv6 enable

		ipv6 enable dev <vif_name>

  Enables IPv6 protocol on the interface, create link-local address using eui-64 scheme

 * ipv6 disable

		ipv6 disable dev <vif_name>

  Disables IPv6 protocol on the interface. Delete all addresses and routes depending on the interface

