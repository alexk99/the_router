# Link Bonding

Since TheRouter is a DPDK application it supports link bonding (link aggregation).
To use link bonding ports it's enough to create them using command line parameters 
of TheRouter (see the section "Command line options" of <a href="conf_options.md">Configuring TheRouter</a>.

To create link bonding port just add the following the router_run.sh

	--vdev 'eth_bond0,mode=2,slave=0000:02:00.0,slave=0000:02:00.1,xmit_policy=l23'

Detailed description of the all link bonding parameters can be found in the DPDK Documentation
<a href="http://dpdk.org/doc/guides-16.07/prog_guide/link_bonding_poll_mode_drv_lib.html">Link Bonding Poll Mode Driver Library</a>

## Example of using link bonding ports

<img src="http://therouter.net/images/tests/link_bonding.png">

