Table of Contents
=================

   * [Table of Contents](#table-of-contents)
   * [Configuration overview](#configuration-overview)
      * [Command line options](#command-line-options)
         * [--lcores](#--lcores)
         * [-n](#-n)
         * [-w](#-w)
      * [Configuration file commands](#configuration-file-commands)
      * [Configuration file examples](#configuration-file-examples)
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
         * [ip addr add](#ip-addr-add)
         * [ip addr del](#ip-addr-del)
         * [sh ip addr](#sh-ip-addr)
      * [ip route tables](#ip-route-tables)
         * [ip route table add](#ip-route-table-add)
         * [ip route table del](#ip-route-table-del)
         * [sh ip route tables](#sh-ip-route-tables)
      * [ip routes](#ip-routes)
         * [ip route add](#ip-route-add)
         * [ip route del](#ip-route-del)
         * [sh ip route](#sh-ip-route)
      * [U32 sets](#u32-sets)
         * [u32set create](#u32set-create)
         * [u32set destroy](#u32set-destroy)
         * [ipset add](#ipset-add)
         * [ipset del](#ipset-del)
         * [ipset test](#ipset-test)
         * [l2set add](#l2set-add)
         * [l2set del](#l2set-del)
         * [l2set test](#l2set-test)
      * [PBR rules](#pbr-rules)
         * [ip pbr rule add](#ip-pbr-rule-add)
         * [ip pbr rule del](#ip-pbr-rule-del)
         * [ip pbr flush](#ip-pbr-flush)
         * [sh ip pbr rules](#sh-ip-pbr-rules)
      * [ARP](#arp)
         * [arp add](#arp-add)
         * [arp del](#arp-del)
         * [sh arp cache](#sh-arp-cache)
      * [ping](#ping)
         * [ping](#ping-1)
      * [NPF](#npf)
         * [sh npf conndb size](#sh-npf-conndb-size)
         * [sh npf conndb summary](#sh-npf-conndb-summary)
         * [sh npf conndb summary sip](#sh-npf-conndb-summary-sip)
         * [sh npf stat](#sh-npf-stat)
         * [npf clear stat](#npf-clear-stat)
         * [NPF sysctl variables controlling connection tracking state timeouts](#npf-sysctl-variables-controlling-connection-tracking-state-timeouts)
      * [NAT events logging via IPFIX](#nat-events-logging-via-ipfix)
         * [Enabling NAT events](#enabling-nat-events)
         * [Configuring ipfix collector](#configuring-ipfix-collector)
      * [Other commands](#other-commands)
         * [shutdown](#shutdown)
         * [sh ver](#sh-ver)
         * [sh uptime](#sh-uptime)
      * [Router statistic commands](#router-statistic-commands)
         * [sh port general stat](#sh-port-general-stat)
         * [sh port stat](#rcli-sh-port-stat)
         * [sh port xstat](#sh-port-xstat)
         * [sh mbuf stats](#sh-mbuf-stats)
         * [sh stat](#sh-stat)
         * [clear stats](#clear-stats)
      * [Sysctl interface](#sysctl-interface)
         * [sysctl set](#sysctl-set)
         * [sysctl get](#sysctl-get)
      * [IPv6](#ipv6)
         * [ipv6 enable](#ipv6-enable)
         * [ipv6 disable](#ipv6-disable)
         * [sh ipv6 addr](#sh-ipv6-addr)
         * [ipv6 addr add eui-64](#ipv6-addr-add-eui-64)
         * [ipv6 addr add](#ipv6-addr-add)
         * [ipv6 addr link-local](#ipv6-addr-link-local)
         * [ipv6 addr link-local eui-64](#ipv6-addr-link-local-eui-64)
         * [ipv6 addr auto](#ipv6-addr-auto)
         * [ipv6 addr del](#ipv6-addr-del)
         * [sh ipv6 route](#sh-ipv6-route)
         * [ipv6 route add](#ipv6-route-add)
         * [ipv6 route add](#ipv6-route-add-1)
         * [ipv6 route add](#ipv6-route-add-2)
         * [ipv6 route add](#ipv6-route-add-3)
         * [ipv6 route del](#ipv6-route-del)
         * [ipv6 route default auto](#ipv6-route-default-auto)
         * [ipv6 route table add](#ipv6-route-table-add)
         * [ipv6 route table del](#ipv6-route-table-del)
         * [ipv6 pbr rule add prefix](#ipv6-pbr-rule-add-prefix)
         * [ipv6 pbr rule add set](#ipv6-pbr-rule-add-set)
         * [ipv6 pbr rule del](#ipv6-pbr-rule-del)
         * [ipv6 nd ra](#ipv6-nd-ra)
         * [sh ipv6 arp](#sh-ipv6-arp)
         * [ipv6 arp add](#ipv6-arp-add)
         * [ipv6 arp del](#ipv6-arp-del)
         * [icmp6 error msg](#icmp6-error-msg)
         * [sh icmp6 error msg](#sh-icmp6-error-msg)
         * [ipv6 nd ra lifetime](#ipv6-nd-ra-lifetime)
         * [ipv6 nd ra interval](#ipv6-nd-ra-interval)
         * [ipv6 nd ra reachable](#ipv6-nd-ra-reachable)
         * [ipv6 nd ra retrans_timer](#ipv6-nd-ra-retrans_timer)
         * [ipv6 nd ra hop_limit](#ipv6-nd-ra-hop_limit)
         * [ipv6 nd ra prefix add|update](#ipv6-nd-ra-prefix-addupdate)
         * [ipv6 nd ra prefix del](#ipv6-nd-ra-prefix-del)
      * [IPv6 sysctl variables](#ipv6-sysctl-variables)
         * [nd_retrans_timer](#nd_retrans_timer)
         * [fib6_max_route_tables](#fib6_max_route_tables)
         * [fib6_max_routes](#fib6_max_routes)
         * [fib6_max_next_hops](#fib6_max_next_hops)
         * [fib6_max_lpm_tbl8](#fib6_max_lpm_tbl8)
         * [max_num_solicited_node_addrs](#max_num_solicited_node_addrs)
         * [nd_neighbor_cache_size](#nd_neighbor_cache_size)
         * [nd_neighbor_cache_entry_ttl](#nd_neighbor_cache_entry_ttl)
         * [icmp6_packet_rate](#icmp6_packet_rate)
         * [icmp6_transmission_burst](#icmp6_transmission_burst)
         * [icmp6_num_buckets](#icmp6_num_buckets)
         * [max_rtr_solicitation_delay](#max_rtr_solicitation_delay)
         * [dad_attempts](#dad_attempts)
      * [VRRP version 3 (IPV6)](#vrrp-version-3-ipv6)
         * [vrrp create group](#vrrp-create-group)
         * [vrrp group ipv6 add](#vrrp-group-ipv6-add)
         * [vrrp group ipv6 add](#vrrp-group-ipv6-add-1)
         * [vrrp group ipv6 del](#vrrp-group-ipv6-del)
         * [vrrp group prio](#vrrp-group-prio)
         * [vrrp group advert_int](#vrrp-group-advert_int)
         * [vrrp group accept_mode](#vrrp-group-accept_mode)
         * [vrrp group preempt_mode](#vrrp-group-preempt_mode)
         * [sh vrrp](#sh-vrrp)
         * [vrrp group del](#vrrp-group-del)
         * [vrrp group nd ra enable/disable](#vrrp-group-nd-ra-enabledisable)
         * [vrrp group nd ra lifetime](#vrrp-group-nd-ra-lifetime)
         * [vrrp group nd ra interval](#vrrp-group-nd-ra-interval)
         * [vrrp group nd ra reachable](#vrrp-group-nd-ra-reachable)
         * [vrrp group nd ra retrans_timer](#vrrp-group-nd-ra-retrans_timer)
         * [vrrp group nd ra hop_limit](#vrrp-group-nd-ra-hop_limit)
         * [vrrp group nd ra prefix add|update](#vrrp-group-nd-ra-prefix-addupdate)
         * [vrrp group nd ra prefix del](#vrrp-group-nd-ra-prefix-del)

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

## Global parameters

### system_name

A string sysctl variable that defines the system name.

Example

	sysctl set system_name "tr1"

### mac_addr_format

A string sysctl variable that defines the format of convertion
MAC addresses to strings.

Valid values are:

	cisco - xxxx.xxxx.xxxx
	linux - xx:xx:xx:xx:xx:xx
	raw - no delimiters, xxxxxxxxxxxx
	
Example:

	sysctl set mac_addr_format linux

### linux_route_proto

An integer sysctl variable that defines the route proto type 
of linux routes TheRouter creates for subscribers, etc.

Note: FRR 4.0 bgpd redistribute kernel command doesn't see linux routes with proto STATIC,
but it sees BOOT routes.

Valid values are:

	2 - RTPROT_KERNEL
	3 - RTPROT_BOOT
	4 - RTPROT_STATIC
	
Example:

	sysctl set linux_route_proto 3

### mbuf

An integet startup sysctl variable that defines the number of mbufs.
16384 value is a typical number of mbufs that is enough for normal operations.

Example:

	sysctl set mbuf linux 16384

## Virtual interfaces

### vif add

Creates a new virtual L3 interface (VIF)

	vif add name <name> port <port_num> type <type> [svid <svid>] [cvid <cvid>] [flags <flag1,flag2...>] [MTU <mtu_size>]

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

	vif flags <up|down> name <name> flags <flag1,flag2...>

The name parameter is the name of a VIF.

Example: disable/down the dhcp_rel flag of the VIF v20.

	vif flags down name v20 flags dhcp_rel

### vif del

Deletes a VIF

	vif del <name>

### vif car

Defines ingress/egress bandwidth limit for a VIF.
<cir_val> unit is Kbit/s. Zero value is used to delete a limit.

	vif car name <name> ingress cir <cir_val> egress cir <cir_value>

### sh vif

Displays all VIFs

	sh vif

Displays information about a particular VIF

	sh vif <name>

### sh vif counters

	sh vif counters
   
### clear vif counters

	clear vif counters

## IP addresses

### ip addr add

	ip addr add <net>/<mask> dev <vif_name>

### ip addr del

	ip addr del <net>/<mask> dev <vif_name>

### sh ip addr  

	sh ip addr

## ip route tables

### ip route table add

Creates a new routing table.

	ip route table add <route_table_name>

### ip route table del

Deletes a routing table.

	ip route table del <route_table_name>

### sh ip route tables

Displays routing tables.

	sh ip route tables

## ip routes

### ip route add

Creates a connected route.

	ip route add <net>/<mask> dev <vif_name> src <src_ip> [table <table_name>]

Creates a route to a prefix reachable via a gateway.
Note that the IP address of the gateway should be reachable via a directly connected
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

Creates a new U32 set.

	u32set create <u32set_name> size <size> bucket_size <bucket_size>

### u32set destroy

Deletes a U32 set.

	u32set destroy <u32set_name>

### ipset add

Adds an ip address to a U32 set.

	ipset add <u32set_name> <ipv4>

### ipset del

Deletes an ip address from a U32 set.

	ipset del <u32set_name> <ipv4>

### ipset test

Tests whether a u32 set containts an ip address or not.

	ipset test <u32set_name> <ipv4>

### l2set add

Adds an VIF identifier (port, svid, and cvid) to a u32 set.

	l2set add <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set del

Delete a VIF identifier (port, svid, and cvid) from a u32 set.

	l2set del <u32set_name> port <port_number> svid <svid> cvid <cvid>

### l2set test

Tests whether a u32 set containts a VIF identifier or not.

	l2set test <u32set_name> port <port_number> svid <svid> cvid <cvid>

## PBR rules

### ip pbr rule add

Creates a PBR rule to match the traffic originated from one of addresses contained in an ip set.

	ip pbr rule add prio <prio_num> u32set <u32set_name> type "ip" table <route_table_name>

Creates a PBR rule to match the traffic originated from one of VIFs contained in a L2 set.

	ip pbr rule add prio <prio_num> u32set <u32set_name> type "l2" table <route_table_name>

Creates a PBR rule to match the traffic originated from a given network.

	ip pbr rule add prio <prio_num> from <net/mask> <route_table_name>

### ip pbr rule del

Deletes a PBR rule with the given priority.

	ip pbr rule del prio <prio_num>

### ip pbr flush

Deletes all PBR rules.

	ip pbr flush

### sh ip pbr rules

Displays PBR rules.

	sh ip pbr rules

## ARP
   
### arp add

Creates an ARP record.

	arp add <ip> <mac> dev <vif_name> [static]

### arp del

Deletes an ARP record.

	arp del <ip> dev <vif_name>
    
### sh arp cache

Displays content of the ARP cache.

	sh arp cache

## ping

### ping

	rcli ping --help
	Usage: ping [-c,--count count] [-i,--interval interval_in_ms] [-s icmp_payload_size]
	[-f,--dont_frag] [-a,--source_address ip_source_address] [-w,--nowait]
	[-h,--help] destination

## NPF

### npf load

Loads NPF configuration file.
Note that a path to the file should be double-quoted.

	npf load "<path_to_npf_configuration_file>"

### sh npf conndb size

	sh npf conndb size
   
### sh npf conndb summary

	sh npf conndb summary

### sh npf conndb summary sip

	sh npf conndb summary sip

### sh npf stat

	sh npf stat

### npf clear stat

	npf clear stat

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

	shutdown

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

	sh port general stat
	port 0
	        pkts rx 2701128
	        pkts tx 2221199
	        bytes rx 1586718600
	        bytes tx 1540072355
	        errors rx 0
	        errors tx 0
	        missed 0
	        rx_nombuf 0

### sh port stat

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

	sysctl set <name> <value>

for string variables:

	sysctl set <name> "<value>"

### sysctl get

Display a sysctl variable value.

	sysctl get <name> <value>

## IPv6

### ipv6 enable

Enables IPv6 protocol on an interface, create link-local address using the eui-64 scheme.

	ipv6 enable dev <vif_name>

### ipv6 disable

Disables IPv6 protocol on the interface. Deletes all addresses and routes depending on the interface.

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

### ipv6 addr auto

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

Enables or disables Router Advertisements at an interface.
If disabled router will not transmit Router Advertisement messages at an interface
and will not answer to Router Solicitation messages.

	ipv6 nd ra enable|disable dev <vif_name>

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

Displays the state of an ICMP error message.

	sh icmp6 error msg type <number> code <number>

### ipv6 nd ra lifetime

Configures the value of the lifetime field of Router Advertisement messages sent 
from an interface.

	ipv6 nd ra lifetime <number> dev <vif_name>

### ipv6 nd ra interval

Configures the MinRtrAdvInterval and MaxRtrAdvInterval values (seconds)
See https://tools.ietf.org/html/rfc4861#page-40
6.2.1.  Router Configuration Variables.

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

## Flow accounting IPFIX. 

### flow ipfix_collector

Configures flow accounting ipfix collector address and port. Default port value is 4739.

	flow ipfix_collector addr <ipv4 address> [port <port-number>]

### sh flow stat

Shows flow accounting statistic counters.

	sh flow stat

## enabling flow accounting on an interface

To enable flow accounting on a particular VIF use the VIF flag "flow_acct". For example:

	vif add name v3 port 2 type dot1q cvid 3 flags npf_on, kni, flow_acct

or use "vif flags" command

	vif flags up name v3 flags flow_acct

### flow_acct

A boolen sysctl variable that defines the flow accounting state.
When enabled flow accouting is generated for forwarded/transit traffic.

	0 - disabled
	1 - enabled

### flow_acct_in_out

A boolean sysctl variable that controls whether to 
generate flow accounting for the input/output traffic or not.

	0 - disabled
	1 - enabled

### flow_acct_dropped_pkts

A boolean sysctl variable that enables flow accounting of dropped packets.

	0 - disabled
	1 - enabled

### flow_idle_timeout

An integer sysctl variable. It defines the idle timeout of traffic flow in seconds.
When the idle timeout expires a traffic flow is exported.

### flow_active_timeout

An integer sysctl variable that defines the active timeout of traffic flow in seconds.
When the active timeout expires a traffic flow is exported.

### flow_ipv4_max

An integer sysctl variable that defines the maximum number of concurrent ipv4 flows entries.

### flow_ipv6_max

An integer sysctl variable that defines the maximum number of concurrent ipv6 flows entries.

### flow_ipv4_worker_max

An integer sysctl variable that defines the maximum number of ipv4 flows 
entries that a worker lcore can process concurrently.

### flow_ipv6_worker_max

An integer sysctl variable that defines the maximum number of ipv6 flows entries 
that a worker lcore can process concurrently.

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

### vif acl create

Creates a new access control list.

	vif acl create aclid <acl_id> type <acl_type> <action>

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

Destroys the ACL with the given ID.

	vif acl destroy aclid <acl_id>

Parameters:
 - acl_id - unique numeric identificator of a ACL to delete

Example:

	vif acl destroy aclid 10

### vif acl add

Apply an ACL to an interface. The command adds an ACL to the ingress or egress list of ACLs of an interface
at position with number prio.

	vif acl add dev <vif_name> dir <direction> aclid <acl_id> prio <prio>

Parameters:
 - vif_name - the name of the interface to add the ACL to;
 - direction - specifies the interface list of ACLs to add to
 Can be one the two values: ingress or egress;
 - acl_id - the numeric identificator of the ACL to add to the interface;
 - prio - position in the interface's ACL list to put the ACL at
 
Example:

	vif acl add dev v5 dir ingress aclid 10 prio 30

### vif acl del

Removes an ACL from an interface. The command deletes an ACL
from the ingress or egress list of ACLs of an interface.

	vif acl del dev <vif_name> dir <direction> aclid <acl_id>

Parameters:
 - vif_name - the name of the interface to delete the ACL from;
 - direction - specifies the interface list of ACLs to delete from.
 Can be one the two values: ingress or egress;
 - acl_id - the numeric identificator of the ACL to delete
 
Example:

	vif acl del dev v5 dir ingress aclid 10

### vif acl mod

Changes the position of an ACL in the list of ACLs on an interface

	vif acl modify dev <vif_name> dir <direction> aclid <acl_id> prio <prio>

Parameters:
 - vif_name - the name of the interface;
 - direction - specifies the interface list of ACLs.
 Can be one the two values: ingress or egress;
 - acl_id - the numeric identificator of the ACL to modify;
 - prio - a new position of the ACL;

Example:

	vif acl modify dev v5 dir ingress aclid 10 prio 40

### vif acl flush

Deletes all rules from an ACL.

	vif acl flush aclid <acl_id>

Parameters:
 - acl_id - the numeric identificator of the ACL to delete rules from
 
Example:

	vif acl flush aclid 10

### vif acl rule add

Adds a rule to an ACL.

	vif acl rule <ip_version> add aclid <acl_id> prio <prio> [proto <protocol_number>]
	[src <src_prefix>] [dst <dst_prefix>] [sport <src_port_range>] [dport <dst_port_range>] 

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6;
 - acl_id - the numeric identificator of the ACL to add the rule to;
 - prio - position of the rule in the ACL
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

Deletes a rule from an ACL.

	vif acl rule <ip_version> del aclid <acl_id> prio <prio>

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6
 - acl_id - numeric identificator of an ACL to add a rule to
 - prio - position of a rule in the ACL
 
Example:

	vif acl rule ipv4 del aclid 11 prio 21

### vif acl rule mod

Modify a rule in an ACL.

	vif acl rule <ip_version> modify aclid <acl_id> prio <prio> [proto <protocol_number>]
	[src <src_prefix>] [dst <dst_prefix>] [sport <src_port_range>] [dport <dst_port_range>] 

Parameters:
 - ip_version - version of the IP protocol. Can be on of two values: ipv4 or ipv6
 - acl_id - the numeric identificator of the ACL to add the rule to
 - prio - position of the rule in the ACL
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

Outputs ACL rules.

	sh vif acl rules aclid <acl_id>

Parameters:
 - acl_id - numeric identificator of an ACL to show
 
Example:

	sh vif acl rules aclid 11
		acl id 11, type ipv4_tuple, action deny, num rules 1
		--
		prio 21, proto any, src any, dst 10.1.1.0/24, sport any, dport 81

### sh vif

Outputs interfaces and ACL applied to them

	sh vif
 
Example:

	# rcli sh vif
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

Outputs connected/online pppoe subscribers

	sh pppoe subsc

### sh pppoe subsc

Outputs the particular pppoe subscriber with the given user-name

	sh pppoe subsc <user-name>

Example:

	sh pppoe subsc alexk
	vif_id  username        mac     svid    cvid    session_id      ip addr mtu     ingress cir     egress cir      rx_pkts tx_pkts rx_bytes        tx_bytes        uptime
	5       alexk   84:16:F9:BD:54:F7       0       3       1       10.11.12.30     1492    100000  100000  182057  269266  18608751        328714627       2 hour(s), 58 min(s), 7 sec(s)

### pppoe disconnect

Disconnects the pppoe subscriber with id <pppoe_vif_id>

	pppoe disconnect <pppoe_vif_id>

### Enable PPPoE at VIF

Enables PPPoE protocol on the interface. See "vif add" for details.

Example:

	vif add name v3 port 0 type dot1q cvid 3 flags flow_acct,pppoe_on,npf_on


### pppoe ac_cookie key

	pppoe ac_cookie key "key_data"

Sets ac_cookie key value.

Example:

	pppoe ac_cookie key "13071232717"

### pppoe ac_name

Sets PPPoE AC name.

	pppoe ac_name "ac_name"

Example:

	pppoe ac_name "trouter1"

### pppoe service name

Sets PPPoE service name.

	pppoe service name "service_name"

Example:

	pppoe service name "*"

### pppoe blocked subsc add

Adds the subscriber link-layer address to the block list.

	pppoe blocked subsc add <lladdr>

### pppoe blocked subsc del

Deletes the subscriber link-layer address from the block list.

	pppoe blocked subsc del <lladdr>

### sh pppoe blocked subsc

Outputs the content of the PPPoE subscriber block list

	sh pppoe blocked subsc

### ppp dns primary

Sets the ip address of the primary DNS server for ppp subscribers.

	ppp dns primary <ip_address>

Example:

	ppp dns primary 8.8.8.8

### ppp dns secondary

Sets the ip address of the secondary DNS server for ppp subscribers.

	ppp dns secondary <ip_address>

Example:

	ppp dns primary 8.8.4.4

### ppp ipcp server ip

Sets the ip address of TheRouter side ot PPP p-t-p tunnels

	ppp ipcp server ip <ip_address>

Example:

	ppp ipcp server ip 10.10.1.1

### ppp ip pool add

Configures the PPP stack to use the local ip pool with the given name.
The pool will be used if all others ip address sources are failed.
For example, the pool will be used if an authorization response doesn't
include Framed-ip-address or Framed-pool atributes.

Multiple pools can be added to ppp. In that case they will
be used in a round-robin way.

	ppp ip pool add <pool_name>

### ppp ip pool del

Stops using the local ip pool that was configured with the command
"ppp ip pool add"

	ppp ip pool del <pool_name>

## PPPoE sysctl variables

### pppoe_max_subsc

The maximum number of concurrent pppoe subscribers.

This variable can be used only in the startup configuration file section.

### pppoe_max_online_subsc

Maximum number of online pppoe subscribers.
Once the number of online pppoe subscribers reaches this
limit TheRouter will stop answering to the pppoe discovery initiation
packets (PADI). The difference between this variable and pppoe_max_subsc
variable is that pppoe_max_online_subsc variable can be changed at runtime.
Use 0 value to turn off the limit.

### pppoe_inactive_ttl

Time in seconds. A PPPoE subscriber will be disconnected
if there are no packets during this period of time.

### tcp_mss_fix

1 - on, 0 - off. Enables or disables using TCP MSS fix for pppoe traffic.

### ppp_max_terminate

The maximum number of PPP FSM (LCP or NCP(IPCP)) terminate packets that may be sent.

### ppp_max_configure

The maximum number of PPP FSM (LCP or NCP(IPCP)) configure packets that may be sent.

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

A booean sysctl variable. When enabled instructs TheRouter to strip 
the domain part of the PPP username before output the username to CLI or log files.

### pppoe_sub_uptime_in_sec

An integer sysctl variable. When enabled TheRouter outputs 
the PPPoE subscriber uptime in seconds instead of a human-readable time format.

### ppp_auth_max_peer_name_len

An integers read-only sysctl variable that stores 
the maximum length of the PPP peers name (usernames) supported by TheRouter.

### ppp_default_auth_proto

A string sysctl variable that defines the default PPP auth proto.
The default value is "chap".

Valid values:

	chap
	pap

Example

	sysctl set ppp_default_auth_proto "chap"

## PPPoE IPv6

### sh pppoe ipv6 subsc

Outputs connected/online PPPoE IPv6 subscribers

	sh pppoe ipv6 subsc

### sh pppoe ipv6 subsc <name>

Output the PPPoE subscriber with the given name

	sh pppoe ipv6 subsc <name>

Example:

	rcli sh pppoe ipv6 subsc alexk
	vif_id  username        mac     svid    cvid    session_id      ia_na   ia_pd   slaac   mtu     ingress cir     egress cir      rx_pkts tx_pkts rx_bytes        tx_bytes        uptime
	5       alexk   84:16:F9:BD:54:F7       0       3       1       xxx:237f:ee39:5584:17eb  xxx60::/64 xxxaf::/64 1492    100000  100000  182736  26991718793821

### dhcpv6 add dns

Adds a recursive DNS server to the global list of servers.
Values on the list will be used in the RA messages sent to PPPoE subsribers and 
by DHCPv6 server. 

	dhcpv6 add dns <ipv6_address>

### dhcpv6 del dns

Deletes a recursive DNS server address from the RDNSS list.

	dhcpv6 del dns <ipv6_address>

### dhcpv6 domain search list

	dhcpv6 domain search list <string>

Example

	dhcpv6 domain search list i6.therouter.net

### ppp ipv6 pool

Sets the default address pool for PPP IPv6 addresses of a particular type

	ppp ipv6 pool <ppp_address_type> <pool_name>

Where <address_type> can take one of the following values:

	ia_na
	ia_pd
	slaac

Example

	# default pools
	ppp ipv6 pool ia_na ppp6_na_pool
	ppp ipv6 pool ia_pd ppp6_pd_pool
	ppp ipv6 pool slaac ppp6_slaac_pool

### ppp ipv6 pool <ppp_address_type> disable

Unsets the default address pool for PPP adresses of a particular type

	ppp ipv6 pool <ppp_address_type> disable

Example:

	ppp ipv6 pool ia_na disable
	ppp ipv6 pool ia_pd disable
	ppp ipv6 pool slaac disable

## PPP IPv6 sysctl variables

### ppp_ipv6

A startup bolean sysctl variable that enables/disables the IPv6 protocol for PPP subscribers	

### ppp_ra_mtu

An integer sysctl variables that defines the size of the MTU Router Advertisement option 
of RA messages sent to PPP interfaces.

### ipv6_tcp_mss_fix

A bolean sysctl variable that enables/disables the MSS fix/clumping for IPv6.

### ppp_dhcpv6_ia_na

An integer sysctl variable that defines the IA_NA option behavior of the PPP DHCPv6 server.

Valid values are:

	0 - disable, the IA_NA option is not included in DHCPv6 messages;
	1 - enable, the IA_NA option is included to the DHCPv6 replies, IA_NA value will be allocated
	from a pool only if the DHCPv6 client asks for that option;
	2 - allways allocate, the IA_NA option is included in the DHCPv6 replies, IA_NA value will be allocated
	from a pool immidiately after the PPP subscriber has connected;

### ppp_dhcpv6_ia_pd

Setup the IA_PD for PPP DHCPv6 server.

Valid values are:

	0 - disable, the IA_PD option is not included in the DHCPv6 messages;
	1 - enable, the IA_PD option is included in the DHCPv6 reply messages, the IA_PD value will be allocated
	from a pool only if the DHCPv6 client asks for that option;
	2 - allways allocate, the IA_PD option is included in the DHCPv6 replies messages,
	the IA_PD value will be allocated from a pool immidiately after the PPP subscriber has connected;

### ppp_slaac

A boolean sysctl varialbe that enables/disables the use of SLAAC for the PPP interfaces.

0 - disable
1 - enable

### dhcpv6_preferred_lt

An integer sysctl variable that defines the default preferred value for IPv6 addresses. 
For example, This value will be used for IPv6 addresses received via RADIUS protocol.

### dhcpv6_valid_lt

An integer sysctl variable that defines the default valid value for IPv6 addresses. 
For example, this value will be used for IPv6 addresses received via RADIUS protocol.

### ppp_rad_acct_slaac

A boolen sysctl variable that controls whether or not to include 
into the radius accounting start messages the Framed-IPv6-Prefix attribute 
carrying the PPP subscriber's SLAAC prefix. 

0 - disable
1 - enable

### ppp_rad_acct_ia_na

A boolen sysctl variable that controls whether or not to include
into the radius accounting start messages
the Framed-IPv6-Address attribute with the PPP subscriber's IA_NA address.

0 - disable
1 - enable


### ppp_rad_acct_ia_pd

A boolen sysctl variable that controls whether or not to include 
into the radius accounting start messages the Delegated-IPv6-Prefix attribute 
carrying the PPP subscriber's IA_PD prefix

0 - disable
1 - enable


## IPv6 Pools

### sh ipv6 pool

Outputs details about IPv6 pools

Example:

	rcli sh ipv6 pools
	---
	name ppp6_pd_pool
	address space xxxe::/48
	address/prefix length 64
	preferred lifetime 3600
	valid lifetime 7200
	free 65536
	used 0
	---
	name ppp6_na_pool
	address space xxx1::/64
	address/prefix length 128
	preferred lifetime 3600
	valid lifetime 7200
	free 4294967294
	used 1
	---
	name ppp6_slaac_pool
	address space xxx0b::/48
	address/prefix length 64
	preferred lifetime 3600
	valid lifetime 7200
	free 65535
	used 1

## ipv6 pool add

Create a new IPv6 address pool.

	ipv6 pool add <pool_name> <prefix> length <len> preferred_lt <integer> valid_lt <integer> flags <flag,...>

<prefix> - address space for a pool.
<len> - length of prefixes allocated from a pool.

Flags:
 
- rand - allocate random values;
- cache - after alocation an address/prefix is reserved for the user allocated the value for a valid_lt seconds.
	During this time the user will be given the same address/prefix. After the valid_lt seconds ellapse the address/prefix
	will be returned back to the pool;

Example:

	ipv6 pool add ppp6_slaac_pool xxx::/48 length 64 preferred_lt 3600 valid_lt 7200 flags rand,cache

## ipv6 pool del

Deletes the ipv6 pool with the given name.

	ipv6 pool del <pool_name>

## ipv6 pool modify

Modifies an existing ipv6 pool lifetime values.

	ipv6 pool modify <pool_name> valid_lt <integer> preferred_lt <integer>

