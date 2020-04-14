Table of Contents
=================

   * [Configuration overview](#configuration-overview)
      * [Command line options](#command-line-options)
         * [--lcores](#--lcores)
         * [-n](#-n)
         * [-w](#-w)
      * [Configuration file commands](#configuration-file-commands)
      * [Configuration file example](#configuration-file-example)
         * [TheRouter configuration file example](#therouter-configuration-file-example)
         * [NPF configaration file example](#npf-configaration-file-example)
   * [Commands](#commands)
      * [Startup commands](#startup-commands)
         * [port](#port)
         * [rx_queue](#rx_queue)
         * [sysctl](#sysctl)
      * [Virtual interfaces](#virtual-interfaces)
         * [vif add](#vif-add)
         * [vif flags](#vif-flags)
         * [vif del](#vif-del)
         * [vif car](#vif-car)
         * [sh vif](#sh-vif)
         * [sh vif counters](#sh-vif-counters)
         * [clear vif counters](#clear-vif-counters)
      * [IP addresses](#ip-addresses)

# Configuration overview

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

### --lcores

Map lcore set to physical cpu set

### -n

Set the number of memory channels to use.

### -w

Add a PCI device in white list.

Example of the startup script cmd options:

	the_router --proc-type=primary --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -c $1 -d

Note:
Lcore 0 is used by the TheRouter's control plane function and can be shared with linux.
Other cores are used by TheRouter's data plane functions and should be isolated during the linux starup process by using
the linux kernel command line parameter isolcpus. Otherwise, performance of TheRouter's data plane working threads
might be very low due the context switching.

## Configuration file commands

This commands are stored a configuration file. To configure TheRouter to use a particular configuration
file -c command line option is used.

The configuration file consists of two sections:

 * startup
 * runtime

Each section contains commands. A section line is considered as a single command.
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

## Configuration file examples

### TheRouter configuration file example

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
	  vif add name p0 port 1 type untagged flags npf_on
	  ip addr add 10.0.0.1/24 dev p0
	
	  vif add name p1 port 0 type untagged flags npf_on
	  ip addr add 10.0.1.1/24 dev p1
	
	  ip route add 0.0.0.0/0 via 10.0.1.2 src 10.0.1.1
	
	  npf load "/etc/npf.conf"
	}

### NPF configaration file example

	alg "icmp"
	alg "pptp"
	
	group default {
	  pass final on p0 all
	  pass final on p1 all
	}

# Commands

## Startup commands

The startup command is a command that is used to initilize TheRouter's susbystems and properties 
that can't be modified once TheRouter have started. Such commands can only be used in
the startup section of a configuration file and can't be used with rcli configuration tool.

### port

	port <dpdk_port_number> mtu <mtu_size> tpid <tpid_value> state enabled flags [flag, ...] [bond_slaves <slave>,...]

Flags:

 - qinq_enabled
 - extend_vlan_offload

Examples:

	port 2 mtu 1500 tpid 0x8100 state enabled bond_slaves 0,1

or

	port 0 mtu 1500 tpid 0x8100 state enabled flags qinq_enabled,extend_vlan_offload


### rx_queue

	rx_queue port <dpdk_port_number> queue <queue_number> lcore <lcore>


### sysctl

		sysctl set <name> <value>

## Virtual interfaces

### vif add

Creates a new virtual L3 interface (VIF)

	rcli vif add name <name> port <port_num> type <type> [svid <svid>] [cvid <cvid>] [flags <flag1,flag2...>] [MTU <mtu_size>]

The Type parameter can take one of the following values:

 - untagged
 - dot1q
 - qinq

Flags:

 - npf_on
 - kni
 - proxy_arp
 - flow_acct
 - rpf
 - dhcp_rel

rpf - reverse path filter

The following flags are enabled by default:

 - dhcp_rel

### vif flags

Up/down (enable/disable) a VIF's flag.

	rcli vif flags <up|down> name <name> flags <flag1,flag2...>

The name parameter is the name of a VIF.

Example: disable/down the dhcp_rel flag of the VIF v20.

	rcli vif flags down name v20 flags dhcp_rel

### vif del

Deletes a VIF

	rcli vif del <name>

### vif car

Defines ingress/egress bandwidth limit for a VIF.
<cir_val> unit is Kbit/s. Zero value is used to delete a limit.

	rcli vif car name <name> ingress cir <cir_val> egress cir <cir_value>

### sh vif

Displays all VIFs

	rcli sh vif

Displays information about a particular VIF

	rcli sh vif <name>

### sh vif counters

	rcli sh vif counters
   
### clear vif counters

	rcli clear vif counters

## IP addresses

### ip addr add

	rcli ip addr add <net>/<mask> dev <vif_name>

### ip addr del

	rcli ip addr del <net>/<mask> dev <vif_name>

### sh ip addr  

	rcli sh ip addr

## ip route tables

### ip route table add

Creates a new routing table.

	rcli ip route table add <route_table_name>

### ip route table del

Deletes a routing table.

	rcli ip route table del <route_table_name>

### sh ip route tables

Displays routing tables.

	rcli sh ip route tables

## ip routes

### ip route add

Creates a connected route.

	rcli ip route add <net>/<mask> dev <vif_name> src <src_ip> [table <table_name>]

Creates a route to a prefix reachable via a gateway.
Note that IP address of the gateway should be reachable via a directly connected
route. A directly connected route is created automatically when an ip address is added
to an interface, but only for the main routing table. All routes for additional route
tables should be created manually.

	ip route add <net>/<mask> via <gw_ip> src <src_ip> [table <table_name>]

Creates an unreachable route.

	ip route add <net>/<mask> unreachable [table <table_name>]

### ip route del

Deletes a route from a routing table.

	ip route del <net/mask> [table <table_name>]

### sh ip route

Displays content of a routing table.

	sh ip route [table <name>]

## U32 sets

### u32set create

Creates a U32 set.

	rcli u32set create <u32set_name> size <size> bucket_size <bucket_size>

### u32set destroy

Deletes a U32 set.

	rcli u32set destroy <u32set_name>

### ipset add

Adds an ip address to a U32 set.

	rcli ipset add <u32set_name> <ipv4>

### ipset del

Deletes an ip address from a U32 set.

	rcli ipset del <u32set_name> <ipv4>

### ipset test

Tests whether a u32 set containts an ip address or not.

	rcli ipset test <u32set_name> <ipv4>

### l2set add

Adds an VIF identifier (port, svid, and cvid) to a u32 set.

	rcli l2set add <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set del

Delete a VIF identifier (port, svid, and cvid) from a u32 set.

	rcli l2set del <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set test

Tests whether a u32 set containts a VIF identifier or not.

	rcli l2set test <u32set_name> port <port_number> svid <svid> cvid <cvid>

## PBR rules

### ip pbr rule add

Creates a PBR rule to match the traffic originated from one of addresses contained in an ip set.

	ip pbr rule add prio <prio_num> u32set <u32set_name> type "ip" table <route_table_name>

Creates a PBR rule to match the traffic originated from one of VIFs contained in a L2 set.

	ip pbr rule add prio <prio_num> u32set <u32set_name> type "l2" table <route_table_name>

Creates a PBR rule to match the traffic originated from a given network.

	rcli ip pbr rule add prio <prio_num> from <net/mask> <route_table_name>

### ip pbr rule del

Deletes a PBR rule with the given priority.

	rcli ip pbr rule del prio <prio_num>

### ip pbr flush

Deletes all PBR rules.

	rcli ip pbr flush

### sh ip pbr rules

Displays PBR rules.

	rcli sh ip pbr rules

## ARP
   
### arp add

Creates an ARP record.

	rcli arp add <ip> <mac> dev <vif_name> [static]

### arp del

Deletes an ARP record.

	rcli arp del <ip> dev <vif_name>
    
### sh arp cache

Displays content of the ARP cache.

	rcli sh arp cache

## ping

### ping

	rcli ping --help
	Usage: ping [-c,--count count] [-i,--interval interval_in_ms] [-s icmp_payload_size]
	[-f,--dont_frag] [-a,--source_address ip_source_address] [-w,--nowait]
	[-h,--help] destination

## NPF

### sh npf conndb size

	rcli sh npf conndb size
   
### sh npf conndb summary

	rcli sh npf conndb summary

### sh npf conndb summary sip

	rcli sh npf conndb summary sip

### sh npf stat

	rcli sh npf stat

### npf clear stat

	rcli npf clear stat

### NPF sysctl variables controlling connection tracking state timeouts

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

	runtime {
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

## NAT events logging via IPFIX

### Enabling NAT events 

	sysctl set ipfix_nat_events 1

### Configuring ipfix collector

	ipfix_collector addr 192.168.20.2

## Other commands

### shutdown

	rcli shutdown

### sh ver

Displays the version of TheRouter software.

	sh ver

### sh uptime

Displays the uptime of TheRouter process.

	sh uptime

## Router statistic commands

### sh port general stat

Displays port's packet counters.

Example:

	rcli sh port general stat
	port 0
	        pkts rx 2701128
	        pkts tx 2221199
	        bytes rx 1586718600
	        bytes tx 1540072355
	        errors rx 0
	        errors tx 0
	        missed 0
	        rx_nombuf 0

### rcli sh port stat

Displays packet counters grouped by lcores.

Example:

	# rcli sh port stat
	port    send_failed     send_ok
	lcore 1: 0      0       1556192
	lcore 2: 0      0       437540
	lcore 3: 0      0       227333
	lcore 4: 0      0       0
	lcore 5: 0      0       0
	lcore 6: 0      0       0
	lcore 7: 0      0       0
	lcore 8: 0      0       0
	lcore 9: 0      0       0
	lcore 10: 0     0       0
	lcore 11: 0     0       0
	lcore 12: 0     0       0
	lcore 13: 0     0       0
	lcore 14: 0     0       0
	lcore 15: 0     0       0
	total: 0        0       2221065

### sh port xstat

Displays port's extended packet counters.

Example:

	# rcli sh port xstat
	port 0
	        rx_good_packets 2702535
	        tx_good_packets 2221993
	        rx_good_bytes 1586895940
	        tx_good_bytes 1540186436
	        rx_q0packets 2702535
	        rx_q0bytes 1586895940
	        tx_q0packets 2221993
	        tx_q0bytes 1540034116
	        mac_local_errors 3
	        mac_remote_errors 1
	        rx_size_64_packets 350936
	        rx_size_65_to_127_packets 1050314
	        rx_size_128_to_255_packets 257371
	        rx_size_256_to_511_packets 47232
	        rx_size_512_to_1023_packets 32387
	        rx_size_1024_to_max_packets 964295
	        rx_broadcast_packets 35939
	        rx_multicast_packets 98145
	        rx_total_packets 2702819
	        rx_total_bytes 1586951636
	        tx_total_packets 2221993
	        tx_size_64_packets 18027
	        tx_size_65_to_127_packets 1035247
	        tx_size_128_to_255_packets 155297
	        tx_size_256_to_511_packets 27883
	        tx_size_512_to_1023_packets 21215
	        tx_size_1024_to_max_packets 964324
	        tx_multicast_packets 1016
	        tx_broadcast_packets 62
	        rx_l3_l4_xsum_error 8513
	        out_pkts_untagged 2221993

### sh mbuf stats

Displays the mbuf usage.

Example:

	# rcli sh mbuf stats
	socket 0 mbuf: free 14450, allocated 1934

### sh stat

Displays global statistics counters.

Example:

	# rcli sh stat
	dropped local                   0
	dropped unknown_vif             43027
	dropped invalid_ipv4            0
	dropped dst unreachable         12
	dropped blackhole               52084
	dropped bad_port                0
	dropped dst_mac_is_unknown      4
	dropped npf_in                  0
	dropped npf_out                 7003
	blocked by npf_in               0
	blocked by npf_out              0
	dropped other_errs              0
	local pkts in                   92539
	local pkts out                  0
	local UDP pkts in               5578
	local UDP pkts out              0
	local pkts dropped              0
	fragmentation overflow          0
	no ctrl mbuf left               0
	no mbuf left                    0
	no timer left                   0
	worker-cplane ring overflow     0
	invalid ipv6 pkts               0

### clear stats

Clears global statistic counters.

## Sysctl interface

### sysctl set

Sets a sysctl variable value.

	rcli sysctl set <name> <value>

for string variables:

	rcli sysctl set <name> "<value>"

### sysctl get

Display a sysctl variable value.

	rcli sysctl get <name> <value>

## IPv6

### ipv6 enable

Enables IPv6 protocol on an interface, create link-local address using the eui-64 scheme.

	ipv6 enable dev <vif_name>

### ipv6 disable

Disables IPv6 protocol on an interface. Deletes all addresses and routes depending on the interface.

	ipv6 disable dev <vif_name>

### sh ipv6 addr

Displays ipv6 addresses assigned to interfaces.

	sh ipv6 addr

### ipv6 addr add eui-64

Generates an address using the given prefix and the interface id (EUI64 scheme) and assigns 
the address to the interface.

	ipv6 addr add eui-64 <prefix>/<length> eui-64 dev <vif_name>

### ipv6 addr add

Assigns an ipv6 address to an interface.

	ipv6 addr add <address>/<length> dev <vif_name>

### ipv6 addr link-local

Assigns the given ipv6 link-local address to an interface.

	ipv6 addr link-local <address> dev <vif_name>

### ipv6 addr link-local eui-64

Generates a link-local address using the eui-64 scheme and assigns it to an interface.

	ipv6 addr link-local eui-64 dev <vif_name>

## ipv6 addr auto

Enables SLAAC client on an interface. Once enabled TheRouter starts assigning dynamic 
ipv6 addresses on an interface based on the information received in Router Advertisement messages.

	ipv6 addr auto dev <vif_name> enable|disable

### ipv6 addr del

Deletes an address from an interface.

	ipv6 addr del <address or prefix>/<length> dev <vif_name>

### sh ipv6 route

Displays content of an ipv6 routing table.

	sh ipv6 route [table <rt_table_name>]

### ipv6 route add

Adds a connected route to a prefix into a routing table.

	ipv6 route add <prefix/prefix-length> dev <vif_name> [table <ipv6_routing_table>]

### ipv6 route add

Adds a route to a prefix reachable via a gateway into a routing table.

	ipv6 route add <prefix/length> dev <vif_name> via <ipv6-address> [table <ipv6_routing_table>]

### ipv6 route add

Adds the default route into a routing table.

	ipv6 route add ::/0 via <ipv6-address> [table <ipv6_routing_table>]

### ipv6 route add

Adds an unreachable route into a routing table.

	ipv6 route add <prefix/length> unreachable [table <ipv6_routing_table>]

### ipv6 route del

Deletes a route from a routing table.

	ipv6 route del <prefix/length> [table <ipv6_routing_table>]

### ipv6 route default auto

Enables installation of the default route based on the information from Router Advertisement messages.
Once enabled TheRouter will create the default route based on the first RA message received on the interface
and will associate a timer with that route. The timer will be set to RA.lifetime value. When it expires the default
route will be deleted and TheRouter will be ready to install a new default route as soon as it receives 
another RA message.

	ipv6 route default auto dev <vif_name> enable|disable

### ipv6 route table add

Adds an new ipv6 route table to the FIB.

	ipv6 route table add <route table name>

### ipv6 route table del

Deletes an ipv6 route table from the FIB.

	ipv6 route table del <route table name>

### ipv6 pbr rule add prefix

Adds an ipv6 PBR rule to the rule list at the given position "rule priority number".

	ipv6 pbr rule add prio <rule priority number> from <ipv6 prefix/length> table <route table name>

### ipv6 pbr rule add set

Adds an ipv6 PBR rule to the rule list at the position "rule priority number".
Only 'l2' value of the 'set type' parameter is supported so far 

	ipv6 pbr rule add prio <rule priority number> u32set <set name> <set type> table <route table name>

### ipv6 pbr rule del

Deletes an ipv6 PBR rule from the rule list from the position "rule priority number".

	ipv6 pbr rule del prio <rule priority number>

### ipv6 nd ra

		ipv6 nd ra enable|disable dev <vif_name>

  Enables or disables Router Advertisements at an interface.
  If disabled router will not transmit Router Advertisement messages at an interface
  and will not answer to Router Solicitation messages

### sh ipv6 arp

Displays ipv6 neighbor cache entries.

	sh ipv6 arp

### ipv6 arp add

Creates or alters an ipv6 neighbor cache entry.

	ipv6 arp add <ipv6-address> <mac-address> dev <vif_name> [static]

### ipv6 arp del

Deletes an ipv6 neighbor cache entry.

	ipv6 arp del <ipv6-address> dev <vif_name>

### icmp6 error msg

Enables or disables generation of ICMP messages with the given type and code.

	icmp6 error msg type <number> code <number> enable|disable

### sh icmp6 error msg

Displays the state of icmp error message.

	sh icmp6 error msg type <number> code <number>

### ipv6 nd ra lifetime

Configures the value of the lifetime field of Router Advertisement messages sent 
from an interface.

	ipv6 nd ra lifetime <number> dev <vif_name>

### ipv6 nd ra interval

Configures the MinRtrAdvInterval and MaxRtrAdvInterval values (seconds)
See https://tools.ietf.org/html/rfc4861#page-40
6.2.1.  Router Configuration Variables

	ipv6 nd ra interval <min_number> <max_number> dev <vif_name>

### ipv6 nd ra reachable

Configures the value to be placed in the Reachable Time field
in the Router Advertisement messages sent by the router.
The value zero means unspecified (by this router).  
MUST be no greater than 3,600,000 milliseconds (1 hour).  

	ipv6 nd ra reachable <number> dev <vif_name>

### ipv6 nd ra retrans_timer

Configures the value to be placed in the Retrans Timer field
in the Router Advertisement messages sent by the router.  
The value zero means unspecified (by this router).

	ipv6 nd ra retrans_timer <number> dev <vif_name>

### ipv6 nd ra hop_limit

Configures the default value to be placed in the Cur Hop Limit
field in the Router Advertisement messages sent by
TheRouter.  The value should be set to the current
diameter of the Internet.  The value zero means
unspecified (by this router).

	ipv6 nd ra hop_limit <number> dev <vif_name>

### ipv6 nd ra prefix add|update

Adds or updates a prefix to/in Router Advertisement messages sent from an interface.

	ipv6 nd ra prefix add|update <prefix/length> [valid_lt <number>] 
	  [preferred_lt <number>] [flags O,A] dev <vif_name>
  
### ipv6 nd ra prefix del

Deletes a prefix from Router Advertisement messages sent from an interface.

	ipv6 nd ra prefix add <prefix/length> dev <vif_name>

## IPv6 sysctl variables

### nd_retrans_timer

The time in milliseconds between retransmissions of Neighbor
Solicitation messages to a neighbor when
resolving the address or when probing the
reachability of a neighbor.

### fib6_max_route_tables

Max number of ipv6 routing tables

### fib6_max_routes

Max number of ipv6 route entries

### fib6_max_next_hops

Max number of ipv6 next hop entries

### fib6_max_lpm_tbl8

Max number of lpm6 tbl8. See https://doc.dpdk.org/guides/prog_guide/lpm6_lib.html

### max_num_solicited_node_addrs

Max number of solicited node addresses

### nd_neighbor_cache_size

Size of the ipv6 neighbor cache.

### nd_neighbor_cache_entry_ttl

ipv6 neigbour cache entry time to live. Seconds.

### icmp6_packet_rate

Icmpv6 error transmission rate in packets per seconds.

### icmp6_transmission_burst

Max number of icmpv6 error messages that could be send at once.

### icmp6_num_buckets

Num icmp6 buckets.

### max_rtr_solicitation_delay

Number of seconds to delay the transmission of router solicitation messages.

### dad_attempts

Number of attempts for Duplicate address detection algorithm.

## VRRP version 3 (IPV6)

### vrrp create group

Creates a new vrrp group.

	vrrp create group <vrrp_id> dev <vif_name> address-family af_ipv6 version 3  

### vrrp group ipv6 add

Setups or changes ipv6 link-local address of a vrrp3 ipv6 group.

	vrrp group <vrrp_id> dev <vif_name> ipv6 add <link-local ipv6 address>
  
### vrrp group ipv6 add

Adds a secondary ipv6 global address to a vrrp3 ipv6 group.

	vrrp group <vrrp_id> dev <vif_name> ipv6 add <ipv6_address> secondary

### vrrp group ipv6 del

Deletes a secondary ipv6 global address from a vrrp3 ipv6 group.

	vrrp group <vrrp_id> dev <vif_name> ipv6 del <ipv6_address> secondary

### vrrp group prio

Changes the priority of a vrrp group.

	vrrp group <vrrp_id> dev <vif_name> prio <value>

### vrrp group advert_int

Changes the advertisement transmission interval (cetiseconds, 100 centiseconds == 1 sec) of a vrrp group.

	vrrp group <vrrp_id> dev <vif_name> advert_int <value>

### vrrp group accept_mode

Changes the accept_mode of a vrrp group.

	vrrp group <vrrp_id> dev <vif_name> accept_mode <on|off>

### vrrp group preempt_mode

Changes the preempt_mode of a vrrp group.

	vrrp group <vrrp_id> dev <vif_name> preempt_mode <on|off>

### sh vrrp

Displays a vrrp group.

	sh vrrp

### vrrp group del 

Deletes a vrrp group.

	vrrp del group <vrrp_id> dev <vif_name>

### vrrp group nd ra enable/disable

Enables or disables transmission of ND Router Advertisement messages for a VRRP IPV6 group.

	vrrp group <vrrp_id> dev <vif_name> nd ra enable|disable

### vrrp group nd ra lifetime

Configures lifetime field value of Router Advertisement messages sent 
for a VRRP IPV6 group.

	vrrp group <vrrp_id> dev <vif_name> nd ra lifetime <value>

### vrrp group nd ra interval

Configures the MinRtrAdvInterval and MaxRtrAdvInterval values (seconds)
See https://tools.ietf.org/html/rfc4861#page-40
6.2.1.  Router Configuration Variables.

	vrrp group <vrrp_id> dev <vif_name> nd ra interval <min_number> <max_number>

### vrrp group nd ra reachable

Configures the value to be placed in the Reachable Time field
in the Router Advertisement messages sent by TheRouter.
The value zero means unspecified (by this router).  
MUST be no greater than 3,600,000 milliseconds (1 hour).

	vrrp group <vrrp_id> dev <vif_name> nd ra reachable <number>
  
### vrrp group nd ra retrans_timer

Configures the value to be placed in the Retrans Timer field
in the Router Advertisement messages sent by the router.  
The value zero means unspecified (by this router).

	vrrp group <vrrp_id> dev <vif_name> nd ra retrans_timer <number>

### vrrp group nd ra hop_limit

Configures the default value to be placed in the Cur Hop Limit
field in the Router Advertisement messages sent by
the router.  The value should be set to the current
diameter of the Internet.  The value zero means
unspecified (by this router).

	vrrp group <vrrp_id> dev <vif_name> nd ra hop_limit <number>

### vrrp group nd ra prefix add|update

Adds or updates a prefix to/in Router Advertisement messages sent for a VRRP IPV6 group.

	vrrp group <vrrp_id> dev <vif_name> nd ra prefix add|update <prefix/length> [valid_lt <number>] 
	  [preferred_lt <number>] [flags O,A] dev <vif_name>
  
### vrrp group nd ra prefix del

Deletes a prefix from Router Advertisement messages sent for a VRRP IPV6 group.

	vrrp group <vrrp_id> dev <vif_name>  nd ra prefix add <prefix/length> dev <vif_name>

