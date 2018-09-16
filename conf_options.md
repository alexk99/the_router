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

  Enables IPv6 protocol on an interface, create link-local address using eui-64 scheme

 * ipv6 disable

		ipv6 disable dev <vif_name>

  Disables IPv6 protocol on the interface. Delete all addresses and routes depending on an interface

* sh ipv6 addr

		sh ipv6 addr

  Outputs ipv6 addresses assignes to interfaces

 * ipv6 addr add eui-64

		ipv6 addr add eui-64 <prefix>/<length> eui-64 dev <vif_name>

  Generates an address using prefix and interface id (EUI64 scheme) and assign the address to
an interface

 * ipv6 addr add

		ipv6 addr add <address>/<length> dev <vif_name>

  Assigns the given ipv6 address to an interface

 * ipv6 addr link-local

		ipv6 addr link-local <address> dev <vif_name>

  Assigns the given ipv6 link-local address to an interface

 * ipv6 addr link-local eui-64

		ipv6 addr link-local eui-64 dev <vif_name>

  Generates a link-local address using the eui-64 scheme and assigns it to an interface

 * ipv6 addr auto

		ipv6 addr auto dev <vif_name> enable|disable

  Enables SLAAC client for an interface. Once enabled the router starts assigning dynamic 
  ipv6 addresses at an interface based on the information received in Router Advertisement messages

 * ipv6 addr del

		ipv6 addr del <address or prefix>/<length> dev <vif_name>

  Deletes an address from an interface

 * sh ipv6 route

		sh ipv6 route

  Outputs ipv6 routing table

 * ipv6 route add

		ipv6 route add <prefix/prefix-length> dev <vif_name>

  Adds a connected route to a prefix into the main routing table

 * ipv6 route add

		ipv6 route add <prefix/length> dev <vif_name> via <ipv6-address> [table <ipv6_routing_table>]

  Adds a route to a prefix via a gateway with given address into a routing table

 * ipv6 route add

		ipv6 route add ::/0 via <ipv6-address> [table <ipv6_routing_table>]

  Adds the default route into a routing table

 * ipv6 route add

		ipv6 route add <prefix/length> unreachable [table <ipv6_routing_table>]

  Adds an unrechable route into a routing table

 * ipv6 route del

		ipv6 route del <prefix/length> [table <ipv6_routing_table>]

  Deletes a route from a routing table

 * ipv6 route default auto

		ipv6 route default auto dev <vif_name> enable|disable

  Enables intallation of default routes based on the information in Router Advertisement messages.
  Once enabled the router will create the default route based on the first RA message received on the interface
  and will associate a timer with that route. The timer is set to RA.lifetime value. When it expires the default
  route will be deleted and the router will install a new default route based on another RA message received at the interface.

 * ipv6 route table add

		ipv6 route table add <route table name>

  Add an ipv6 route table to the FIB

 * ipv6 route table del

		ipv6 route table del <route table name>

  Deletes an ipv6 route table from the FIB

 * ipv6 pbr rule add prefix

		ipv6 pbr rule add prio <rule priority number> from <ipv6 prefix/length> table <route table name>

  Adds an ipv6 pbr rule to the rule list at the position "rule priority number"

 * ipv6 pbr rule add set

		ipv6 pbr rule add prio <rule priority number> u32set <set name> <set type> table <route table name>

  Adds an ipv6 pbr rule to the rule list at the position "rule priority number".
  Only "l2" set type value is supported so far. 

 * ipv6 pbr rule del

		ipv6 pbr rule del prio <rule priority number>

  Deletes an ipv6 pbr rule from the rule list from the position "rule priority number"

 * ipv6 nd ra

		ipv6 nd ra enable|disable dev <vif_name>

  Enables or disables Router Advertisements at an interface.
  If disabled router will not transmit Router Advertisement messages at an interface
  and will not answer to Router Solicitation messages

 * sh ipv6 arp

		sh ipv6 arp

  Outputs ipv6 neighbor cache entries

 * ipv6 arp add

		ipv6 arp add <ipv6-address> <mac-address> dev <vif_name> [static]

  Creates or alters an ipv6 neighbor cache entry

 * ipv6 arp del

		ipv6 arp del <ipv6-address> dev <vif_name>

  Deletes an ipv6 neighbor cache entry

 * icmp6 error msg

		icmp6 error msg type <number> code <number> enable|disable

  Enables or disables generation of an icmp messages with the given type and code

 * sh icmp6 error msg

		sh icmp6 error msg type <number> code <number>

  Outputs state of icmp error message

 * ipv6 nd ra lifetime

		ipv6 nd ra lifetime <number> dev <vif_name>

  Configures lifetime field value of Router Advertisement messages sent 
  from an interface

 * ipv6 nd ra interval

		ipv6 nd ra interval <min_number> <max_number> dev <vif_name>

  Configures the MinRtrAdvInterval and MaxRtrAdvInterval values (seconds)
  See https://tools.ietf.org/html/rfc4861#page-40
  6.2.1.  Router Configuration Variables

 * ipv6 nd ra reachable

		ipv6 nd ra reachable <number> dev <vif_name>

  Configures the value to be placed in the Reachable Time field
  in the Router Advertisement messages sent by the router.
  The value zero means unspecified (by this router).  
  MUST be no greater than 3,600,000 milliseconds (1 hour).  

 * ipv6 nd ra retrans_timer

		ipv6 nd ra retrans_timer <number> dev <vif_name>

  Configures the value to be placed in the Retrans Timer field
  in the Router Advertisement messages sent by the router.  
  The value zero means unspecified (by this router).

 * ipv6 nd ra hop_limit

		ipv6 nd ra hop_limit <number> dev <vif_name>

  Configures the default value to be placed in the Cur Hop Limit
  field in the Router Advertisement messages sent by
  the router.  The value should be set to the current
  diameter of the Internet.  The value zero means
  unspecified (by this router).

 * ipv6 nd ra prefix add|update

		ipv6 nd ra prefix add|update <prefix/length> [valid_lt <number>] 
		  [preferred_lt <number>] [flags O,A] dev <vif_name>

  Adds or updates a prefix to/in Router Advertisement messages sent from an interface
  
 * ipv6 nd ra prefix del

		ipv6 nd ra prefix add <prefix/length> dev <vif_name>

  Deletes a prefix from Router Advertisement messages sent from an interface		

## IPv6 sysctl variables

 * nd_retrans_timer

  The time in milliseconds between retransmissions of Neighbor
  Solicitation messages to a neighbor when
  resolving the address or when probing the
  reachability of a neighbor.

 * fib6_max_route_tables

  Max number of ipv6 routing tables

 * fib6_max_routes

  Max number of ipv6 route entries

 * fib6_max_next_hops

  Max number of ipv6 next hop entries

 * fib6_max_lpm_tbl8

  Max number of lpm6 tbl8. See https://doc.dpdk.org/guides/prog_guide/lpm6_lib.html

 * fib6_max_num_local_ip_addrs

  Max number if ipv6 addresses assigned at interfaces

 * max_num_solicited_node_addrs

  Max number of solicited node addresses

 * nd_neighbor_cache_size

  ipv6 neighbor cache size

 * nd_neighbor_cache_entry_ttl

  ipv6 neigbour cache entry time to live. Seconds.

 * icmp6_packet_rate

  Icmpv6 error transmission rate in packets per seconds.

 * icmp6_transmission_burst

  Max number of icmpv6 error messages that could be send at once

 * icmp6_num_buckets

  Num icmp6 buckets

 * max_rtr_solicitation_delay

  number of seconds to delay the transmission of router solicitation messages

 * dad_attempts

  Number of attempts for Duplicate address detection algorithm

## IPV6 VRRP version 3

 * vrrp create group

		vrrp create group <vrrp_id> dev <vif_name> address-family af_ipv6 version 3

  Creates vrrp group

 * vrrp group ipv6 add

		vrrp group <vrrp_id> dev <vif_name> ipv6 add <link-local ipv6 address>

  Setup or change ipv6 link-local address of a vrrp3 ipv6 group
  
 * vrrp group ipv6 add

		vrrp group <vrrp_id> dev <vif_name> ipv6 add <ipv6_address> secondary

  Add secondary ipv6 global address to a vrrp3 ipv6 group
  
 * vrrp group ipv6 del

		vrrp group <vrrp_id> dev <vif_name> ipv6 del <ipv6_address> secondary

  Del secondary ipv6 global address from a vrrp3 ipv6 group

 * vrrp group prio

		vrrp group <vrrp_id> dev <vif_name> prio <value>

  Change priority of a vrrp group

 * vrrp group advert_int

		vrrp group <vrrp_id> dev <vif_name> advert_int <value>

  Change advertisement transmission interval (cetiseconds, 100 centiseconds == 1 sec) of a vrrp group

 * vrrp group accept_mode

		vrrp group <vrrp_id> dev <vif_name> accept_mode <on|off>

  Change accept_mode of a vrrp group

 * vrrp group preempt_mode

		vrrp group <vrrp_id> dev <vif_name> preempt_mode <on|off>

  Change preempt_mode of a vrrp group
  
 * sh vrrp

		sh vrrp

  Shows vrrp group information
  
 * vrrp group del 

		vrrp del group <vrrp_id> dev <vif_name>

  Deletes vrrp group
  
 * vrrp group nd ra enable/disable

		vrrp group <vrrp_id> dev <vif_name> nd ra enable|disable

  Enables or disables transmission of ND Router Advertisement messages for a VRRP IPV6 group

 * vrrp group nd ra lifetime

		vrrp group <vrrp_id> dev <vif_name> nd ra lifetime <value>

  Configures lifetime field value of Router Advertisement messages sent 
  for a VRRP IPV6 group

 * vrrp group nd ra interval

		vrrp group <vrrp_id> dev <vif_name> nd ra interval <min_number> <max_number>

  Configures the MinRtrAdvInterval and MaxRtrAdvInterval values (seconds)
  See https://tools.ietf.org/html/rfc4861#page-40
  6.2.1.  Router Configuration Variables

 * vrrp group nd ra reachable

		vrrp group <vrrp_id> dev <vif_name> nd ra reachable <number>

  Configures the value to be placed in the Reachable Time field
  in the Router Advertisement messages sent by the router.
  The value zero means unspecified (by this router).  
  MUST be no greater than 3,600,000 milliseconds (1 hour).
  
 * vrrp group nd ra retrans_timer

		vrrp group <vrrp_id> dev <vif_name> nd ra retrans_timer <number>

  Configures the value to be placed in the Retrans Timer field
  in the Router Advertisement messages sent by the router.  
  The value zero means unspecified (by this router).

 * vrrp group nd ra hop_limit

		vrrp group <vrrp_id> dev <vif_name> nd ra hop_limit <number>

  Configures the default value to be placed in the Cur Hop Limit
  field in the Router Advertisement messages sent by
  the router.  The value should be set to the current
  diameter of the Internet.  The value zero means
  unspecified (by this router).

 * vrrp group nd ra prefix add|update

		vrrp group <vrrp_id> dev <vif_name> nd ra prefix add|update <prefix/length> [valid_lt <number>] 
		  [preferred_lt <number>] [flags O,A] dev <vif_name>

  Adds or updates a prefix to/in Router Advertisement messages sent for a VRRP IPV6 group
  
 * vrrp group nd ra prefix del

		vrrp group <vrrp_id> dev <vif_name>  nd ra prefix add <prefix/length> dev <vif_name>

  Deletes a prefix from Router Advertisement messages sent for a VRRP IPV6 group
