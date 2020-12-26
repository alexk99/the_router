# 1. Install and configure FRR or Quagga

follow this installation steps
https://github.com/alexk99/the_router/blob/master/frr_quagga_install.txt

also take a look here for more details
https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite

# 2. Configure TR

## 2.1.
add following commands to the startup section of TR's configuration file

    sysctl set linux_route_proto 3
    sysctl set ppp_install_subsc_linux_routes 1

## 2.2. 
use the "kni" flag for all TR VIF interfaces that will be used to communicate with external routers via a dynamic routing protocol

for example

    # uplink interface
    vif add name v3 port 0 type dot1q cvid 3 flags kni

## 2.3.
you should manually set up the linux KNI interfaces after TR has started.
to do that use ip link command:

    $rvrf ip link set up dev r_xxx

where xxx is the name of your KNI VIF interface.
you can place this command in file therouter_start.sh in the section

    ##
    ## Setup KNI interfaces
    ##

# 3. Output subscriber's routes
Once a subscriber has connected to TR
TR will create a linux /32 route for that subscriber.
The route will be added to the 'tr' linux namespace at the dev 'lo'.
To output such routes use

    $rvrf ip route ls dev lo

Don't forget to define $rvrf bash variable as

    ip netns exec tr

# 4. Configure FRR
Use FRR command 'redistribute kernel' to redistribute routes created by TR to your uplink router.

