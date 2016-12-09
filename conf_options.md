# Configuration of The router

There are three groups of configuration options and commands of The router:

 * command line options
 * configuration file commands
 * runtime commands
	
## Command line options
Command line options can be modified by editing the run script /usr/local/sbin/router_run.sh.
This are mostly DPDK EAL command line options, therefore for detailed description of them you can
refer to DPDK documentation <a href="http://dpdk.org/doc/guides/testpmd_app_ug/run_app.html?highlight=eal%20options">EAL Command-line Options</a>

Before running the router you must check the following options and use your own values depending on the hardware you use:
 * -c
Set the hexadecimal bitmask of the cores to run on.
	
 * --lcores
Map lcore set to physical cpu set

 * -n
Set the number of memory channels to use.

 * -w
Add a PCI device in white list.

Example of the startup script cmd options:
	```
	the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -c $1 -d
	```

Note:
Lcore 0 will be used for the router's control plane task and can be shared with any linux tasks.
The other cores will be used in the router's data plane process and you should isolate them during the linux starup process by using
linux kernel command line parameters isolcpus. Otherwise due the context switching performance of the routers working threads
could be very low.

## Configuration file options

This options are stored in the /etc/router.conf file.

Configuration file commands consists of the two groups:
 * startup
 * runtime

	```
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
	```

## Configuration file example
```
startup {
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
```

### Startup commands 
are the commands that can't be modified once the router have started.
So the only way to set them up is to edit the configuration file /etc/router.conf.

 * port
	```
	port <dpdk_port_number> mtu <mtu_size> tpid <tpid_value> state enabled
	```
 
 * rx_queue
 	```
 	rx_queue port <dpdk_port_number> queue <queue_number> lcore <lcore>
 	```
 	
 * sysctl
 ```
 sysctl set <name> <value>
 ```
 
 * npf load
 ```
 npf load "<path_to_npf_configuration_file>"
 ```
 Note: that you should enclose path to file with '"'.

### Runtime commands 
are the commands that can be executed while a router is working.
You could also use any of this commands in the runtime section of the router configuarion file but without rcli prefix.

### ip addr
 * ip addr add
	```
	rcli ip addr add <net>/<mask> dev <vif_name>
	```
	
 * ip addr del
	```
	rcli ip addr del <net>/<mask> dev <vif_name>
	```
   
 * sh ip addr  
	```
	rcli sh ip addr
	```

### ip route
 * ip route add
	```
	rcli ip route add <net>/<mask> dev <vif_name> src <src_ip> [table <table_name>]
	or
	ip route add <net>/<mask> via <gw_ip> src <src_ip> [table <table_name>]
	or
	ip route add <net>/<mask> unreachable [table <table_name>]
	```
   
 * ip route del
   ```
   ip route del <net/mask> [table <table_name>]
   ```
   
 * sh ip route
   ```
   rcli sh ip route
   ```
   
### vif
 * vif add
   ```
   rcli vif add name <name> port <port_num> type <type> [svid <svid>] [cvid <cvid>] [flags <flag1,flag2...>]
   ```
   
   Type parameter can take one of the following values:
     - untagged
     - dot1q
     - qinq
   
   Flags:
   	 - kni
   	 - proxy_arp
   	   

 * vif del
   ```
   rcli vif del <name>
   ```

   
 * sh vif
   ```
   rcli sh vif
   ```
   
 * sh vif counters
   ```
   rcli sh vif counters
   ```
   
 * clear vif counters
   ```
   rcli clear vif counters
   ```

### arp
   
 * arp add
   ```
   rcli arp add <ip> <mac> dev <vif_name> [static]
   ```
   
 * arp del
   ```
   rcli arp del <ip> dev <vif_name>
   ```
   
 * sh arp cache
   ```
   rcli sh arp cache
   ```

### sysctl

 * sysctl set
   ```
   rcli sysctl set <name> <value>
   ```

 * sysctl set
   ```
   rcli sysctl get <name> <value>
   ```

### ping

 * ping
   ```
	rcli ping --help
	Usage: ping [-c,--count count] [-i,--interval interval_in_ms] [-s icmp_payload_size]
	[-f,--dont_frag] [-a,--source_address ip_source_address] [-w,--nowait]
	[-h,--help] destination
   ```

### NPF
 * sh npf conndb size
   ```
   rcli sh npf conndb size
   ```
   
 * sh npf conndb summary
   ```
   rcli sh npf conndb summary
   ```

 * sh npf stat
   ```
   rcli sh npf stat
   ```

 * npf clear stat
   ```
   rcli npf clear stat
   ```

### Other commands

 * shutdown
   ```
   rcli shutdown
   ```

### Router statistic commands

 * sh port ext stat
   ```
   rcli sh port ext stat
   ```

 * sh port stat
   ```
   rcli sh port stat
   ```

 * sh cmbuf stats
   ```
   rcli sh cmbuf stats
   ```

 * sh mbuf stats
   ```
   rcli sh mbuf stats
   ```
   
 * sh stats
   ```
   rcli sh stats
   ```
   
 * clear stats
   ```
   rcli clear stats
   ```
