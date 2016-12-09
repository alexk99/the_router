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

## Configuration file options

This options are stored in the /etc/router.conf file.

Configuration file commands consists of the two groups:
 * startup
 * runtime

### Startup commands 
are the commands that can't be modified once the router have started.
So the only way to set them up is to edit the configuration file /etc/router.conf.

 * port
 * rx_queue
 * sysctl

### Runtime commands 
are the commands which values can be altered by the rcli utility in any time.

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
   	   
   
 * sh vif
   ```
   rcli 
   ```
   
 * sh vif counters
   ```
   rcli 
   ```
   
   -
   ```
   rcli 
   ```
   
   
   
 * npf load
   