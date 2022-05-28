# Install FRR

Follow installation steps for Ubuntu https://github.com/alexk99/the_router/blob/master/ubuntu_frr.md)

To find out more details about a way FRR is communicating with BisonRouter you can take a look here
https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite

# Configure BisonRouter

## Subscriber routes

To announce PPPoE/IPoE subscriber's /32 routes add following commands to the startup section of a BR's configuration file

    sysctl set linux_route_proto 3
    sysctl set ppp_install_subsc_linux_routes 1

## KNI

Use the "kni" flag on VIF interfaces that are used to communicate with neighbor routers 
via a dynamic routing protocol

for example

    # uplink interface
    vif add name v3 port 0 type dot1q cvid 3 flags kni

Linux KNI interfaces must be set up after BisonRouter has started.
To instruct the bisonrouter script to do that edit the 'br_kni_vifs'
variable in /etc/bisonrouter/bisonrouter.env and include names 
of the KNI interfaces into it.

Note that the KNI interface name consists from a prefix 'r_' and the name 
of the corresponding VIF inteface.

For example:

	br_kni_vifs=(
		"r_v3"
	)

# Output subscriber's routes

Once a subscriber has connected BR will create a linux /32 route for it.
The route will be added to the 'br' linux namespace at the dev 'lo'.
To output such routes use

	export rvrf="ip netns exec br"
 	$rvrf ip route ls dev lo

# Redistribute routes

Use the FRR command 'redistribute kernel' to redistribute routes created by BisonRouter to neighbor routers.

