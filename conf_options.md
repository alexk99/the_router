# Configuration of The router

There are three groups of configuration options of The router.
	- command line options
	- configuration file options
	- runtime options
	
## Command line options
Command line options can be modified by editing the run script /usr/local/sbin/router_run.sh.
This are mostly DPDK EAL command line options, therefore for detailed description of them you can
refer to DPDK documentation <a href="http://dpdk.org/doc/guides/testpmd_app_ug/run_app.html?highlight=eal%20options>EAL Command-line Options</a>

Before running the router you must check the following options and use your own values depending on the hardware you use:
### -c
Set the hexadecimal bitmask of the cores to run on.
	
### --lcores
Map lcore set to physical cpu set

### -n
Set the number of memory channels to use.

### -w
Add a PCI device in white list.

## Configuration file options

## Runtime options