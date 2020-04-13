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

## Configuration file example

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

Create a new virtual L3 interface (VIF)

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

Delete a VIF

	rcli vif del <name>

### vif car

Define ingress/egress bandwidth limit for a VIF.
<cir_val> unit is Kbit/s. Zero value is used to delete a limit.

	rcli vif car name <name> ingress cir <cir_val> egress cir <cir_value>

### sh vif

Output all VIFs

	rcli sh vif

Output information about a particular VIF

	rcli sh vif <name>

### sh vif counters

	rcli sh vif counters
   
### clear vif counters

	rcli clear vif counters

## IP addresses

