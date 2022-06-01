# Captive Portal

BisonRouter Captive Portal redirection function is build on top of the following components:

 * Linux host with kernel DNAT rules;
 * OpenResty Nginx web server;
 * BisonRouter Policy Based Routing;

The scheme works as follows:

- BisonRouter uses the Policy Based Routing (PBR) to redirect traffic
to a Linux host;
- The Linux host uses iptables DNAT rules to redirect HTTP traffic to
an OpenResty Nginx Web server running at the same Linux host;
- OpenResty Nginx Web server requests BisonRouter where it should
redirect HTTP queries. BisonRouter uses 'ipoe captive portal' option
to compose a response and sent it back to OpenResty. Upon receiving
the response OpenResty performs HTTP redirection to a destination
captive portal page located at the address carried in the response.

## Configuring a link to the local Linux host

BisonRouter should have a network connection to the local Linux host
running BisonRouter. Since Linux and BisonRouter are two different
network operating systems the Linux host must use a dedicated NIC.

Let's assume that there is a point-to-point connection
between BisonRouter and Linux with the following IP addresses:

BisonRouter: 192.168.100.1/24
Linux: 192.168.100.2/24

Note: make shure that BisonRouter can reach Linux by using command:
  rcli ping 192.168.100.2

## Installing OpenResty Nginx

    systemctl disable nginx
    systemctl stop nginx
    apt-get -y install --no-install-recommends wget gnupg ca-certificates
    wget -O - https://openresty.org/package/pubkey.gpg | sudo apt-key add -
    echo "deb http://openresty.org/package/ubuntu $(lsb_release -sc) main"| sudo tee /etc/apt/sources.list.d/openresty.list
    apt -y install openresty
    # check
    service openresty status

## Configuring OpenResty

    cp /usr/local/openresty/nginx/conf/nginx.conf /usr/local/openresty/nginx/conf/nginx.conf.old
    cp /etc/bisonrouter/captive_portal/nginx.conf.example /usr/local/openresty/nginx/conf/nginx.conf

Edit /usr/local/openresty/nginx/conf/nginx.conf file and set up the 'listen' option

    listen 192.168.100.2;

Restart OpenResty

    service openresty status

## Configuring BisonRouter

### Routing table 'rt_bl'

BisonRouter should be configured to use an additional routing table
named 'rt_bl' with the default route pointing to the IP address of
OpenResty Nginx Web server (192.168.100.2).

BR's PBR rules must be configured to use the 'rt_bl' table to
route only the traffic of the unuthorized subscribers that should be redirected to
the Captive Portal.

Example

    # create an additional routing table with name "rt_bl"
    ip route table add rt_bl

    # create a connected route to a network with a default router
    ip route add 192.168.100.0/24 dev v20 table rt_bl

    # create a default route
    ip route add 0.0.0.0/0 via 192.168.100.2 table rt_bl

    #
    # 'rt_bl' routing table should also contain
    #   - a route to the Captive Portal Web server
    #   - a route to DNS server used by subscribers
    #

    # ipsets containing subscriber's ip addresses
    #
    # Note: size should be large enouph to store all online
    # subscriber's IP addresses
    #
    u32set create ips1 size 131072 bucket_size 16
    u32set create l2s1 size 131072 bucket_size 16
    subsc u32set init ips1 l2s1

    #
    # PBR rules
    #  forbit by default, ips1 - permit
    #  10.111.0.0/16 - subscriber's subnet
    #
    ip pbr rule add prio 10 u32set l2s1 type "l2" table main
    ip pbr rule add prio 20 u32set ips1 type "ip" table main
    ip pbr rule add prio 30 from 10.111.0.0/16 table rt_bl

## Configuring the Linux host

### DNAT

Incoming HTTP request's destination address should be rewritten
with the ip address of the OpenResty Nginx server 192.168.100.2.

    iptables -A PREROUTING -t nat -p tcp -i vlan20 --dport 80  -j DNAT --to 192.168.100.2:80

Note: vlan20 is a linux host interface to a p-t-p network to BR.

### Routing
The linux host must have an ip route to BisonRouter subscriber networks,
in order for OpenResty Nginx 302 redirect replies to reach subscribers.

    ip route add 10.111.0.0/16 via 192.168.100.2/24

Note:
  the iptable DNAT rule and the route to subscribers must
  be saved to rc.local in order to autostart them after the reboot.

## Controlling subscriber Internet access. Radius attributes.

    #
    # PBR rules
    #  forbit by default, ips1 - permit
    #  10.111.0.0/16 - subscriber's subnet
    #
    ip pbr rule add prio 10 u32set l2s1 type "l2" table main
    ip pbr rule add prio 20 u32set ips1 type "ip" table main
    ip pbr rule add prio 30 from 10.111.0.0/16 table rt_bl

The PBR rule with prio 30 instructs BR to redirect subscriber's traffic
to the captive portal. This happens only if ipset 'ips1' doesn't contain that 
particular subscriber's IP address.

Therefore to conroll subscriber internet access the following logic must be implemented
in a Radius/OSS server:

- upon connection, a subscriber's IP address must be deleted from the ipset 'ips1'
in order to make sure the subscriber doesn't have internet access until the captive
portal grants it such permission. Therefore radius authentication reply must contain 
an attribute/value pair therouter_pbr=2.

- to grant a subscriber internet access after the
subscriber has been authorized by the Captive Portal subscriber's IP
address must be added to the ipset 'ips1'. 
This could be accomplished either by using rcli command

    rcli ipset add ips1 x.x.x.x

or by sending to BR a CoA message carring the following radius attribute/value pair
to add subscriber's ip address to the ipset "rt_bl"

    therouter_pbr=1

### Other commands:

    # delete IP address 10.11.1.12 from 'ips1'
    rcli ipset del ips1 10.11.1.12

    # output the content of 'ips1'
    rcli sh ipset ips1
