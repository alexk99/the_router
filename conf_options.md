# Configuration

There are three groups of configuration options and commands:

 * command line options
 * configuration file commands
 * rcli commands
	
# Command line options
Command line options can be modified by editing the run script /usr/local/sbin/router_run.sh.
Most of them are DPDK EAL command line options, therefore you can
refer to DPDK documentation <a href="http://dpdk.org/doc/guides/testpmd_app_ug/run_app.html?highlight=eal%20options">EAL Command-line Options</a>
for detailed description of them.

Before running TheRouter you must check the following options and use your own values depending on the hardware you use:

 * -c

Set the hexadecimal bitmask of the cores to run on.

 * --lcores

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

# Configuration file options

This options are stored a configuration file used with -c command line option.

Configuration file consists two sections:

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

## Configuration file example

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

## NPF configaration file example

	group default {
	  pass final on p0 all
	  pass final on p1 all
	}

# Startup commands

Startup commands are the commands that are used to initilize TheRouter's susbystem and properties 
that can't be modified once TheRouter have started. This commands can only be used in
the startup section of the configuration file and can't be used by the rcli interface.

## port

		port <dpdk_port_number> mtu <mtu_size> tpid <tpid_value> state enabled

## rx_queue

	 	rx_queue port <dpdk_port_number> queue <queue_number> lcore <lcore>

## sysctl

		sysctl set <name> <value>

## npf load

	 	npf load "<path_to_npf_configuration_file>"

 
 Note: that you should enclose path to file with '"'.

# Runtime commands 
Runtime commands are the commands that can be either executed via rcli interface or used in the runtime section of
the configuration file.

## ip addr

### ip addr add

		rcli ip addr add <net>/<mask> dev <vif_name>

### ip addr del

		rcli ip addr del <net>/<mask> dev <vif_name>

### sh ip addr  

		rcli sh ip addr

## ip route tables

### ip route table add

		rcli ip route table add <route_table_name>

### ip route table del

		rcli ip route table del <route_table_name>

### rcli sh ip route tables

		rcli rcli sh ip route tables

## U32 sets

### u32set create

		rcli u32set create <u32set_name> size <size> bucket_size <bucket_size>

### u32set destroy

		rcli u32set destroy <u32set_name>

### ipset add

		rcli ipset add <u32set_name> <ipv4>

### ipset del

		rcli ipset del <u32set_name> <ipv4>

### ipset test

		rcli ipset test <u32set_name> <ipv4>

### l2set add

		rcli l2set add <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set del

		rcli l2set del <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set test

		rcli l2set test <u32set_name> port <port_number> svid <svid> cvid <cvid>


## PBR rules

### sh ip pbr rules

		rcli sh ip pbr rules

### ip pbr rule add

		ip pbr rule add prio <prio_num> u32set <u32set_name> type "ip" table <route_table_name>
		
		ip pbr rule add prio <prio_num> u32set <u32set_name> type "l2" table <route_table_name>
		
		rcli ip pbr rule add prio <prio_num> from <net/mask> <route_table_name>

### ip pbr rule del

		rcli ip pbr rule del prio <prio_num>

### ip pbr flush

		rcli ip pbr flush


## ip route

### ip route add

		rcli ip route add <net>/<mask> dev <vif_name> src <src_ip> [table <table_name>]

	or

		ip route add <net>/<mask> via <gw_ip> src <src_ip> [table <table_name>]

	or

		ip route add <net>/<mask> unreachable [table <table_name>]

### ip route del

		ip route del <net/mask> [table <table_name>]

### sh ip route

		sh ip route [table <name>]

## vif

### vif add

	rcli vif add name <name> port <port_num> type <type> [svid <svid>] [cvid <cvid>] [flags <flag1,flag2...>] [MTU <mtu_size>]

Type parameter can take one of the following values:

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

	rcli vif flags <up|down> name <name> flags <flag1,flag2...>

Example:

	rcli vif flags down name v20 flags dhcp_rel

Up/down (enable/disable) a flag on a VIF.

### vif car

	rcli vif car name <name> ingress cir <cir_val> egress cir <cir_value>

Define ingress/egress bandwidth limit for a VIF.
<cir_val> unit is kbit/s. a zero value could be used
to delete a limit.

### vif del

	rcli vif del <name>

### sh vif

	rcli sh vif
   
### sh vif counters

	rcli sh vif counters
   
### clear vif counters

	rcli clear vif counters

## arp
   
### arp add

	rcli arp add <ip> <mac> dev <vif_name> [static]

### arp del
 
	rcli arp del <ip> dev <vif_name>
    
### sh arp cache
 
	rcli sh arp cache

## sysctl

### sysctl set

	rcli sysctl set <name> <value>

for string variables

	rcli sysctl set <name> "<value>"

### sysctl get

	rcli sysctl get <name> <value>

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

## NAT events logging via IPFIX

### Enable nat events 

	sysctl set ipfix_nat_events 1


### Setup ipfix collector

	ipfix_collector addr 192.168.20.2

## Other commands

### shutdown

	rcli shutdown

## Router statistic commands

### sh port ext stat

	rcli sh port ext stat

### sh port stat
 
	rcli sh port stat

### sh cmbuf stats

	rcli sh cmbuf stats

### sh mbuf stats

	rcli sh mbuf stats

### sh stat

	rcli sh stat
   
### clear stats
 
	rcli clear stats

# Sysctl variables

### system_name

Router system name.

Example

	sysctl set system_name "tr1"

### mac_addr_format

String. Defines the format of convertion
MAC addresses to strings.

Valid values are:

	cisco - xxxx.xxxx.xxxx
	linux - xx:xx:xx:xx:xx:xx
	raw - no delimiters, xxxxxxxxxxxx
	
Example:

	rcli sysctl set mac_addr_format linux

### linux_route_proto

Integer. Defines the route proto type 
of linux routes TheRouter create for subscribers, etc.

Note: FRR 4.0 bgpd redistribute kernel doesn't see linux routes with proto static,
but it sees BOOT routes.

Valid values are:

	2 - RTPROT_KERNEL
	3 - RTPROT_BOOT
	4 - RTPROT_STATIC
	
Example:

	rcli sysctl set linux_route_proto 3


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

## Flow accounting (IPFIX)

### sysctl variables

use 'sysctl get <varname>' or 'sysctl set <varname> <varvalue>' command
to view or modity a sysctl variable. 

 * flow_acct

    Flow account state.
    When enabled flow accouting is generated for the forwarded/transit traffic.

		0 - disabled
		1 - enabled

 * flow_acct_in_out

    Generate flow accounting for the input/output traffic.

		0 - disabled
		1 - enabled

 * flow_acct_dropped_pkts

    If enabled do flow accounting for dropped packets.

		0 - disabled
		1 - enabled

 * flow_idle_timeout

    Idle timeout of a flow, secs. When the idle timeout expires an idle flow is exported.

 * flow_active_timeout

    active timeout of a flow, secs. When the active timeout expires an active flow is exported.

 * flow_ipv4_max

    maximum number of concurrent ipv4 flows entries

 * flow_ipv6_max

    maximum number of concurrent ipv6 flows entries

 * flow_ipv4_worker_max

    maximum number of ipv4 flows entries that a single lcore (worker) can process
    concurrently

 * flow_ipv6_worker_max

    maximum number of ipv6 flows entries that a single lcore (worker) can process
    concurrently

## rcli commands

 * flow ipfix_collector

		flow ipfix_collector addr <ipv4 address> [port <portnumber>]

  Configures flow accounting ipfix collector address and port. Default port value is 4739.

 * sh flow stat

		sh flow stat

  Show flow accounting statistic counters.


## Enabling flow accounting on an interface
To enable flow accounting on a particular VIF use VIF flag "flow_acct". For example:

	vif add name v3 port 2 type dot1q cvid 3 flags npf_on, kni, flow_acct

## Access control lists (ACL)

Access control lists can be used to filter traffic incoming to an interface (ingress) or
outging from an interface. Multiple ACL can be applied to the same interface and a single ACL
can be applied to many interfaces. 

Each interface has two ACL list: ingress and egress. An interface ACL list stores ACL
in sorted order. The position of ACL in the list is defined by a priority specified by a user.

An ACL, in turn, consists of rules. The position of a rule in an ACL is also defined by priority.

When a packet comes into an interface it is compared to the rules of ACLs from the ingress list,
when a packet is transmitted from an interface it is compared to the rules of ACLs from the egress list.
First, a packet is compared to the rules of the ACL with the minimum priority. Then the process 
goes to the next ACL with greater priority.

When a packet is matched to an ACL rule, the ACL process is stopped and the action defined
by the ACL is taken to the packet. It could be a drop or permit action.

If a packet is not matched to any ACL rule the process goes to the next ACL in the list.
If there are no more ACL in the list, then the action opposite to the action defined by ACL is taken.
For example, if a packet is not matched to any rule of a deny ACL, then the packet is permitted.
And when a packet isn't matched to any rule of a permit ACL the packet is dropped.
So, the last ACL in the list defines the fate of a packet when no matches are found.
Note that empty ACLs are not included into interfaces list of ACL and won't be taken
into account when a packet isn't matched to any ACL rules. 

## RCLI commands

### vif acl create

		vif acl create aclid <acl_id> type <acl_type> <action>

Creates new access control list.

Parameters:
 - acl_id - unique numeric identificator
 - acl_type - type. It can be one of the following types:
   - ipv4_tuple - define an ACL that can classify packets using combination 
of following fields: protocol type, ipv4 source address, ipv4 destination address, 
l4 source port, l4 destination port
   - ipv6_tuple - define an ACL that can classify packets using combination 
of following fields: protocol type, ipv6 source address, ipv6 destination address, 
l4 source port, l4 destination port
 - action: action to take for a packet when a match is found. It can be one of the following
 values:
   - deny - drop a packet when a match is found
   - permit - permit a packet when a match is found

Example:

		vif acl create aclid 10 type ipv6_tuple deny


### vif acl destroy

		vif acl destroy aclid <acl_id>

Deletes an access control list.

Parameters:
 - acl_id - unique numeric identificator of a ACL to delete

Example:

		vif acl destroy aclid 10

### vif acl add

		vif acl add dev <vif_name> dir <direction> aclid <acl_id> prio <prio>

Use an ACL on an interface. Command adds an ACL to the ingress or egress list of ACLs of an interface
at position number prio.

Parameters:
 - vif_name - name of an interface to add an ACL to
 - direction - specifies the interface list of ACLs to add an ACL to.
 Can be one the two values: ingress or egress
 - acl_id - numeric identificator of the ACL to add to an interface
 - prio - position in the interface's ACL list to put an ACL to
 
Example:

		vif acl add dev v5 dir ingress aclid 10 prio 30

### vif acl del

		vif acl del dev <vif_name> dir <direction> aclid <acl_id>

Stop using an ACL on an interface. Command deletes an ACL
from the ingress or egress list of ACLs of an interface.

Parameters:
 - vif_name - name of an interface to delete an ACL from
 - direction - specifies the interface list of ACLs to delete an ACL from.
 Can be one the two values: ingress or egress
 - acl_id - numeric identificator of the ACL to delete from an interface
 
Example:

		vif acl del dev v5 dir ingress aclid 10

### vif acl mod

		vif acl modify dev <vif_name> dir <direction> aclid <acl_id> prio <prio>

Changes the position of an ACL in the list of ACLs on an interface

Parameters:
 - vif_name - name of an interface
 - direction - specifies the interface list of ACLs.
 Can be one the two values: ingress or egress
 - acl_id - numeric identificator of the ACL to modify
 - prio - new position of an ACL
 
Example:

		vif acl modify dev v5 dir ingress aclid 10 prio 40

### vif acl flush

		vif acl flush aclid <acl_id>

Deletes all rules from an ACL.

Parameters:
 - acl_id - numeric identificator of the ACL to delete rules from
 
Example:

		vif acl flush aclid 10

### vif acl rule add

		vif acl rule <ip_version> add aclid <acl_id> prio <prio> [proto <protocol_number>]
		[src <src_prefix>] [dst <dst_prefix>] [sport <src_port_range>] [dport <dst_port_range>] 

Adds a rule to an ACL.

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6
 - acl_id - numeric identificator of an ACL to add a rule to
 - prio - position of a rule in the ACL
 - proto - ip protocol number
 - src_prefix - source ip prefix
 - dst_prefix - destination ip prefix
 - src_port_range - l4 source port range: for example: 8080 8090
 - dst_port_range - l4 destination port range
 
Examples:

		vif acl rule ipv4 add aclid 11 prio 21 proto 6 src 10.1.0.0/24 dst 10.2.0.0/24 sport 10 20 dport 80
		or
		vif acl rule ipv4 add aclid 11 prio 21 dst 10.1.0.0/24 dport 80
		or
		vif acl rule ipv6 add aclid 10 prio 20 dst 2a00:1450:400c:c07::8b dport 80

### vif acl rule del

		vif acl rule <ip_version> del aclid <acl_id> prio <prio>

Deletes a rule from an ACL.

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6
 - acl_id - numeric identificator of an ACL to add a rule to
 - prio - position of a rule in the ACL
 
Example:

		vif acl rule ipv4 del aclid 11 prio 21

### vif acl rule mod

		vif acl rule <ip_version> modify aclid <acl_id> prio <prio> [proto <protocol_number>]
		[src <src_prefix>] [dst <dst_prefix>] [sport <src_port_range>] [dport <dst_port_range>] 

Modify a rule in an ACL.

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6
 - acl_id - numeric identificator of an ACL to add a rule to
 - prio - position of a rule in the ACL
 - proto - ip protocol number
 - src_prefix - source ip prefix
 - dst_prefix - destination ip prefix
 - src_port_range - l4 source port range: for example: 8080 8090
 - dst_port_range - l4 destination port range
 
Examples:

		vif acl rule ipv4 modify aclid 11 prio 21 proto 6 src 10.1.0.0/24 dst 10.2.0.0/24 sport 10 20 dport 80
		or
		vif acl rule ipv4 modify aclid 11 prio 21 dst 10.1.0.0/24 dport 80
		or
		vif acl rule ipv6 modify aclid 10 prio 20 dst 2a00:1450:400c:c07::8b dport 80
		
### sh vif acl rules

		sh vif acl rules aclid <acl_id>

Shows ACL.

Parameters:
 - acl_id - numeric identificator of an ACL to show

 
Example:

		h5 ~ # rcli sh vif acl rules aclid 11
		acl id 11, type ipv4_tuple, action deny, num rules 1
		--
		prio 21, proto any, src any, dst 10.1.1.0/24, sport any, dport 81

### sh vif

		sh vif

Shows ACL interface.
 
Example:

		h5 ~ # rcli sh vif
		vif v5, id 3
		  port 0, vlan 0.5, encapsulation dot1q
		  mac address 00:1B:21:A3:0C:88
		  NPF index 12
		  CAR ingress not set
		      egress not set
		  ACL ingress prio 30 acl 10, prio 40 acl 11
		      egress not set

## PPPoE subscribers

### sh pppoe subsc

	sh pppoe subsc

Output all connected/online pppoe subscribers

### sh pppoe subsc

	sh pppoe subsc <user-name>

Output particular pppoe subscriber with the given user-name

Example:

	h5 ~ # $rvrf rcli sh pppoe subsc
	vif_id  mac     session_id      ip addr mtu     ingress cir     egress cir      tx_pkts rx_pkts
	9       60:A4:4C:41:0A:24       6       10.11.1.1       1480    50      55      0       0

### pppoe disconnect

	pppoe disconnect <pppoe_vif_id>

Disconnect pppoe subscriber VIF with id <pppoe_vif_id>

### Enable PPPoE on VIF

See "vif add" for all details.

Example:

	vif add name v3 port 0 type dot1q cvid 3 flags flow_acct,pppoe_on,npf_on

Using pppoe_on flag enables PPPoE protocol at an interface.

### pppoe ac_cookie key

	pppoe ac_cookie key "key_data"

Sets ac_cookie key value.

Example:

	pppoe ac_cookie key "13071232717"

### pppoe ac_name

	pppoe ac_name "ac_name"

Sets PPPoE AC name.

Example:

	pppoe ac_name "trouter1"

### pppoe service name

	pppoe service name "service_name"

Sets PPPoE service name.

Example:

	pppoe service name "*"

### pppoe blocked subsc add

	pppoe blocked subsc add <lladdr>

Add pppoe subscriber link-layer address to the block list

### pppoe blocked subsc del

	pppoe blocked subsc del <lladdr>

Del pppoe subscriber link-layer address from the block list

### sh pppoe blocked subsc

	sh pppoe blocked subsc

Show content of the pppoe subscriber's block list

### ppp dns primary

	ppp dns primary <ip_address>

Sets ip address of the primary dns server for ppp subscribers.

Example:

	ppp dns primary 8.8.8.8

### ppp dns secondary

	ppp dns secondary <ip_address>

Sets ip address of the secondary dns server for ppp subscribers.

Example:

	ppp dns primary 8.8.4.4

### ppp ipcp server ip

	ppp ipcp server ip <ip_address>

Sets ip address the_router side ot ppp p-t-p tunnels

Example:

	ppp ipcp server ip 10.10.1.1

### ppp ip pool add

	ppp ip pool add <pool_name>

Configure ppp to use local ip pool with the given name.
The pool will be used if all others ip address sources are failed.
For example the pool will be used if the authorization response doesn't
include Framed-ip-address or Framed-pool atributes.

Multiple pools can be added to ppp. In that case they will
be used in a round-robin way.

### ppp ip pool del

	ppp ip pool del <pool_name>

Stop using the local ip pool that was configured with the command
"ppp ip pool add"

## PPPoE subsribers sysctl variables

### pppoe_max_subsc

Maximum number of concurrent pppoe subscribers.

Variable can be used only in the startup configuration file section.

### pppoe_max_online_subsc

Maximum number of online pppoe subscribers.
Once the number of online pppoe subscribers reaches this
limit therouter will stop answering to pppoe discovery initiation
packets (PADI). The difference between this variable and pppoe_max_subsc
variable is that pppoe_max_online_subsc variable can be changed at runtime.
Use 0 value to turn off the limit.

### pppoe_inactive_ttl

Time, seconds. Pppoe subscriber will be disconnected
if there are no packets during this period of time.

### tcp_mss_fix
1 - on, 0 - off. Enables or disables using TCP MSS fix for pppoe traffic.

### ppp_max_terminate
Maximum number of PPP FSM (LCP or NCP(IPCP)) terminate packets that may be sent.

### ppp_max_configure
Maximum number of PPP FSM (LCP or NCP(IPCP)) configure packets that may be sent.

### ppp_initial_restart_time
Time is milliseconds to wait before resend a PPP FSM configure request.
This time will be multiplied by 1.5 with each attempt to resend a request.

### pppoe_sub_uniq_check
1 - on, 0 - off. Check that each pppoe subscriber has a uniq pair: Host-Uniq TAG and MAC address.
If a new pppoe discover request containging already existing pair of the values is received
it will be dropped. The pppoe_sub_uniq_check variable can be used only in the startup configuration file section.

### ppp_1session_per_username
1 - on, 0 - off.
When enabled the_router will disconnect/prevent from connecting new pppoe subscribers that use a username
already being used by another subscriber's session

### ppp_install_subsc_linux_routes
1 - on, 0 - off.
When enabled the_router will add/remove linux kernel /32 routes for ppp subscribers ip addresses.
Linux kernel routes are installed to 'lo' interface in the namespace therouter is running in.
This option allows to announce subscriber's /32 prefixes by using "redisribute kernel" command
in FRR/Quagga bgpd or ospfd daemons. 

### lcp_keepalive_interval
Interval in seconds beetween keepalive tests.
Use 0 to turn off keeplive.

### lcp_keepalive_probe_interval
Interval in milliseconds beetween keepalive probes (LCP echo-requests).

### lcp_keepalive_num_retries
Maximum number of keepalive probes that could be send during
one keepalive test.

### ppp_username_strip_domain_cli_log

Strip the domain part of ppp username before output a username
to CLI or log files.

### pppoe_sub_uptime_in_sec

Show PPPoE subscriber's uptime in seconds instead of a human-readable time format.

### ppp_auth_max_peer_name_len

Read only. Indicates maximum length of ppp peers names (usernames) allowed
by TheRouter.

### ppp_default_auth_proto

String. Defines the default ppp auth proto.
Default values is "chap".

Valid values:

	chap
	pap

Example

	sysctl set ppp_default_auth_proto "chap"


## IP pools

### ip pool add

	ip pool add <ip_pool_name>

Add new ip pool with the given name.
Maximum pool name length is 16.

### ip pool del

	ip pool del <ip_pool_name>

Delete ip pool with the given name.
Maximum pool name length is 16.

### ip pool add range

	ip pool add range <ip_pool_name> <ip from> - <ip to>

Add ip address range to the pool.
Maximum pool name length is 16.

### ip pool del range

	ip pool del range <ip_pool_name> <ip from> - <ip to>

Delete ip address range from the pool.
Maximum pool name length is 16.

### sh ip pool

	sh ip pool

Output ip pool detailes

## IP pools sysct variables

### ippool_cache_size

Size of the ip pool's glabal pool cache.
Default value is 4096.

### ippool_cache_entry_ttl

Time to live of the ip pool cache entry, seconds.
Default value is 60 seconds.

## RADIUS and CoA

### radius_client add server

	radius_client add server <ip address> [port <port number>]

Add RADIUS server to the list of servers. RADIUS requests
will be sent to servers in the round-robin way. Maximum numbers
of servers in the list is 8. Default port number is 1812.

Example:

	radius_client add server 192.168.5.2 port 1612


### radius_client add src ip 

	radius_client add src ip <ip address>

Add ip address to the list of source ip addresses that will be used
by the TheRouter's RADIUS client to send RADIUS requests. A source
ip address must be assigned to a VIF.

Example:

	radius_client add src ip 192.168.5.111



### radius_client set secret

	radius_client set secret "secret"

Set the RADIUS client secret.

Example:

	radius_client set secret "1234abcd"
	


### coa server set secret

	coa server set secret "secret"

Set the RADIUS CoA server secret.

Example:

	coa server set secret "abcd1234"
	
## RADIUS Accounting

### radius_client set secret

	radius_client set accounting secret "secret"

Set the RADIUS accounting client secret.

Example:

	radius_client set accounting secret "1234abcd"


### radius_client add server

	radius_client add accounting server <ip address> [port <port number>]

Add RADIUS accounting server to the list of servers. RADIUS requests
will be sent to servers in the round-robin way. Maximum numbers
of servers in the list is 8. Default port number is 1813.

Example:

	radius_client add accounting server 192.168.5.2 port 1813

### Accounting sysctl variables

#### radius_accounting

  1 - enable radius accounting, 0 - disable rodius accounting
  Note that "vif_stat" sysctl variable should be set to 1, otherwise
  accounting request's packets and bytes counters will contains
  zero values.

#### radius_accounting_interim

  1 - enable sending interim accounting-requests, 0 - disable

#### radius_accounting_interim_interval

  Time in seconds, determines how often to send interim requests.

## Radius attributes

This is the list of TheRouter VAS:

	VENDOR       TheRouter     12345
	BEGIN-VENDOR TheRouter
	    ATTRIBUTE therouter_ingress_cir 1 integer
	    ATTRIBUTE therouter_engress_cir 2 integer
	    ATTRIBUTE therouter_ipv4_addr 3 integer
	    ATTRIBUTE therouter_ipv4_mask 4 integer
	    ATTRIBUTE therouter_outer_vid 5 integer
	    ATTRIBUTE therouter_inner_vid 6 integer
	    ATTRIBUTE therouter_ip_unnumbered 7 integer
	    ATTRIBUTE therouter_port_id 8 integer
	    ATTRIBUTE therouter_ipv4_gw 9 integer
	    ATTRIBUTE therouter_pbr 10 integer
	    ATTRIBUTE therouter_install_subsc_route 17 integer
	    ATTRIBUTE therouter_subsc_ttl 18 integer
	    ATTRIBUTE therouter_subsc_static_arp 19 integer
	    ATTRIBUTE therouter_subsc_proxy_arp 20 integer
	    ATTRIBUTE therouter_subsc_rp_filter 21 integer
	END-VENDOR   TheRouter

#### therouter_ingress_cir

Ingress CIR (commited information rate) - kbit/s
It's used to limit bandwith availabe for any type of subscriber.
Ingress direction means the direction from the subscriber to TheRouter interface. 

#### therouter_egress_cir

Egress CIR (commited information rate) - kbit/s
It's used to limit bandwith availabe for any type of subscriber.
Egress direction means the direction from the TheRouter to the subscriber. 

#### therouter_subsc_ttl

subscriber's session time to live in seconds.

#### therouter_install_subsc_route

Value 1 of this attribute instructs TheRouter to
intstall subscriber ipv4 address /32 prefix into the main 
routing table.

#### therouter_subsc_static_arp

Value 1 of this attribute instructs TheRouter to
intstall the static arp record for L2 subscriber.

#### therouter_subsc_proxy_arp

The attribute with value 1 instructs TheRouter to enable
proxy arp on dynamic VIF (vlan per subscriber).

Note that to enable proxy arp for L2 subscribers
the proxy arp flag should be used in VIF configuration command
when the L2 subscriber's parent interface is created. 

#### therouter_subsc_rp_filter

The attribute with value 1 instructs TheRouter to enable
reverse path filter on dynamic VIF (vlan per subscriber) 

Note that to enable reverse path filter for L2 subscribers
the rpf flag should be used in VIF configuration command
when the L2 subscriber's parent interface is created. 

#### therouter_pbr

Description is available 
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management_eng.md#pbr-rules">here</a>

# IPoE subscribers

## IPoE subsribers sysctl variables

### subsc_vif_max

Maximum number of concurrent ipoe subscribers.
Variable can be used only in the startup configuration file section.

### install_subsc_linux_routes

1 - on, 0 - off.

When enabled the_router will add/remove linux kernel /32 routes for ipoe subscriber's ip addresses.
Linux kernel routes are installed to 'lo' interface in the namespace therouter is running in.
This option allows to announce subscriber's /32 prefixes by using "redisribute kernel" command
in FRR/Quagga bgpd or ospfd daemons. 

### subsc_initiate_by_egress_pkts 

1 - on, 0 - off.

Initiate L2 connected subscriber sessions by unclassified egress packets.

### subsc_initiate_by_ingress_pkts 

1 - on, 0 - off.

Initiate L2 connected subscriber sessions by unclassified ingress packets.

### l2_subsc_initiate_by_dhcp 

1 - on, 0 - off.

Initiate L2 connected subscriber sessions by DHCP ACK messages going
through TheRouter's DHCP Relay subsystem.

### subsc_update_expiration_by_ingress_pkts

1 - on, 0 - off.

Update expiration/TTL time of L2 subscriber by ingress packets.
On by default.

### subsc_update_expiration_by_egress_pkts

1 - on, 0 - off.

Update expiration/TTL time of L2 subscriber by egress packets.
On by default.

### l2_subsc_arp_security

1 - on, 0 - off.

When enabled TheRouter will filter ARP requests at VIFs with flag l2_subsc
and reply only to requests received from authorized L2 subscribers.
TheRouter will be trying to find a match of ARP source ip and ARP source hardware address
to IP and MAC address pair of an L2 subscriber, also it will check that ARP
request has been received from the VIF the matching subscriber is connected to. 
If no matches are found, the request will be ignored.

## DHCP Relay

### Enable DHCP relay

	sysctl set dhcp_relay_enabled 1

### Setup address of DHCP server to relay requests to

	dhcp_relay <ipv4_address>

### setup dhcp option82 rewrite function

	dhcp_relay opt82 mode <mode>

where mode is one of the following values:

 * rewrite_off - turn off option82 rewrite/insert function
 * rewrite_if_doesnt_exist - insert remote_id and circuit_id suboptions
only if the request doesn't already contain dhcp option82
 * rewrite - rewrite or insert both remote_id and circuit_id DHCP option82 suboptions
 * rewrite_circuit_id - rewrite or insert only the circuit_id DHCP option82 suboption
 * rewrite_remote_id - rewrite or insert only the remote_id DHCP option82 suboption

### setup dhcp gateway-ip address (giaddr)

	dhcp_relay giaddr <ipv4>

Set global dhcp realy giaddr address. When defined TheRouter will
use it instead of picking up an ip address with minimum value from
the interface where dhcp request was received.

### set content of the remote_id suboption

	dhcp_relay opt82 remote_id "tr_h4"

### circuit_id suboption format

 TheRouter writes 6 bytes of data into the circuit_id option.
 
 This data include the following properties of the interface a dhcp request was received on:
 
 * 2 bytes - port_id
 * 2 bytes - svlan_id
 * 2 bytes - cvlan_id

### dhcp_relay_opt82_curcuit_id_plain_text

dhcp_relay_opt82_curcuit_id_plain_text is a systcl boolean variable.
Valid values 0 and 1. When enabled therouter uses the plain text format for circuit_id suboption.

# Range commands

Some commands could be applied to a set of interfaces
or create a set of interfaces. Those type of commands
are called 'range' commands in TheRouter terms.

Syntax of the range commands is symilar to the syntax of corresponing
commands that operate with only one VIF only with a exceptions that
range commands have additional range parameter.

The main purpuse of the range commands is create multiple
interfaces with the same parameters but different vlan numbers.


## VIF range commands

### vif add range

	vif add name <name> port <port_num> type <type> range svid <vlan_range> cvid <vlan_range> [flags <flag1,flag2...>] [mtu <mtu>]

Where 'vlan_range' is a vlan number, or a vlan range, for example 

	svid 4 cvid 100 200

would create 101 VIFs from vlan number 4.100 to 4.200

	svid 4 5 cvid 100 200

would create 202 VIFs from vlan number 4.100 to 5.200.
The rest of parameters are the same as for the simple 'vif add' command. 

Example:

	vif add name vlanr port 0 type qinq range svid 2079 cvid 2500 2800 flags l2_subs

### vif del range

	vif del range svid <vlan_range> cvid <vlan_range> name <name>

## IP address range commands

### ip addr add range

	ip addr add svid <vlan_range> cvid <vlan_range> <net>/<mask> name <name>

### ip addr del range

	ip addr del svid <vlan_range> cvid <vlan_range> <net>/<mask> name <name>

## ACL range commands

### vif acl add range

	vif acl add svid <vlan_range> cvid <vlan_range> name <name> dir <direction> aclid <acl_id> prio <prio>

### vif acl del range

	vif acl del svid <vlan_range> cvid <vlan_range> name <name> dir <direction> aclid <acl_id>

### vif acl mod range

	vif acl modify svid <vlan_range> cvid <vlan_range> name <name> dir <direction> aclid <acl_id> prio <prio>

