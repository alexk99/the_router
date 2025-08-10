# DNS redirection

BisonRouter DNS redirection function is build on top of the following components:

 * Linux host with kernel DNAT rules;
 * BIND recursive DNS server;
 * BisonRouter Policy Based Routing (PBR);

The scheme works as follows:

- BisonRouter uses the Policy Based Routing (PBR) to redirect traffic
to a Linux host;
- The Linux host uses iptables DNAT rules to redirect DNS traffic to
a BIND recursive DNS server running at the same Linux host;

<img src="https://files.therouter.net/images/br_rdns_redirection2.png">

## Configuring a link to the local Linux host

BisonRouter should have a network connection to the local Linux host
running BisonRouter. Since Linux and BisonRouter are two different
network operating systems, the Linux host must use a dedicated NIC.

Let's assume that there is a point-to-point connection
between BisonRouter and Linux with the following IP addresses:

    BisonRouter: 192.168.100.1/24
    Linux: 192.168.100.2/24

Note: make sure that BisonRouter can reach Linux by using the command:
  rcli ping 192.168.100.2

## Installing BIND

    sudo apt-get update
    sudo apt-get install bind9 bind9utils bind9-doc
    # status check
    service named status

## Configuring BIND

Edit /etc/bind/named.conf.options.
Note: 10.0.0.0/16 is an example subscriber subnet.

    acl acl_allow {
            10.0.0.0/16;
            localhost;
            localnets;
    };

    options {
            directory "/var/cache/bind";
            dnssec-validation auto;
            listen-on-v6 { none; };
            listen-on { 192.168.100.2; };
            recursion yes;
            allow-query { acl_allow; };
    };

Restart BIND

    service named restart

## Configuring BisonRouter

### Routing table 'rt_dns'

BisonRouter should be configured to use an additional routing table
named 'rt_dns' with the default route pointing to the IP address of
BIND recursive DNS server (192.168.100.2).

BR's PBR rules must be configured to use the 'rt_dns' table to route only the DNS traffic.

Example

    runtime {
        
    # create an additional routing table with the name "rt_dns"
    ip route table add rt_dns

    # create a connected route to a network with a default router
    ip route add 192.168.100.0/24 dev v20 table rt_dns

    # create a default route
    ip route add 0.0.0.0/0 via 192.168.100.2 table rt_dns

    #
    # PBR rule to redirect DNS traffic to the routing table rt_dns
    #
    ip pbr rule add prio 10 proto udp dst port 53 table rt_dns
    
    }

## Configuring the Linux host

### DNAT

The incoming DNS request's destination address should be rewritten
with the IP address of the BIND recursive DNS server 192.168.100.2.

    iptables -A PREROUTING -t nat -p udp -i vlan20 --dport 53  -j DNAT --to 192.168.100.2:53

Note: vlan20 is a Linux host interface to a p-t-p network to BR.

### Routing

The Linux host must have an ip route to BisonRouter subscriber networks,
in order for the BIND replies to reach subscribers.
For example, if your subscriber network is 10.0.0.0/16 and BR IP address
is 192.168.100.1 then a route should be as follows:

    ip route add 10.0.0.0/16 via 192.168.100.1

Note:
  The iptable DNAT rule and the route to subscribers must
  be saved to rc.local in order to autostart them after the linux reboot.
