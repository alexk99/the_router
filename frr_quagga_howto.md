# 1. Install and configure FRR or Quagga

follow this installation steps
https://github.com/alexk99/the_router/blob/master/frr_quagga_install.txt

also take a look here for more details
https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite

# 2. Configure TR

## 2.1.
add following commands to the startup section of BR's configuration file

    sysctl set linux_route_proto 3
    sysctl set ppp_install_subsc_linux_routes 1

## 2.2. 
Use the "kni" flag for all VIF interfaces that will be used to communicate with external routers via a dynamic routing protocol

for example

    # uplink interface
    vif add name v3 port 0 type dot1q cvid 3 flags kni

## 2.3.

Linux KNI interfaces must be set up after BisonRouter has started.
To configure the bisonrouter script to do that edit the 'br_kni_vifs'
variable in /etc/bisonrouter/bisonrouter.env and include KNI interfaces 
names into it.

For example:

br_kni_vifs=(
	"r_v3"
	"r_v100"
)

# 3. Output subscriber's routes
Once a subscriber has connected to TR
TR will create a linux /32 route for that subscriber.
The route will be added to the 'br' linux namespace at the dev 'lo'.
To output such routes use

    $rvrf ip route ls dev lo

Don't forget to define $rvrf bash variable as

    ip netns exec br

# 4. Configure FRR
Use FRR command 'redistribute kernel' to redistribute routes created by BisonRouter to your uplink router.

