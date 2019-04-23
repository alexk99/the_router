# Subscriber management.

## TheRouter supports following features requied for running a BRAS (Broadband Remote Access Server):

 * IPoE L2/L3 connected subscribers
 * IPoE vlan per subsriber with ip unnumbered adresses
 * proxy arp
 * traffic shaping (Token bucket filter with extended burst value)
 * DHCP relay
 * redirect subsribers traffic based on multiple routing tables and PBR
 * radius/coa

# Subsriber types

## L2 connected subscribers

<img src="http://therouter.net/images/bras/l2_connected_subsc_overview.png">

Subscribers are connected to a router via a shared L2 domain.
A router interface is configured as a kind of a parent interface for subscriber sessions.
Subscribers parent interface can use any type of ethernet encapsulations: untagged, dot1q, qinq.
In order to configure a router interface as a subscriber parent interfaces the VIF flag 'l2_subs' 
must be used in the vif configuration command.

	vif add name v_subsc port 0 type qinq svid 2 cvid 121 flags l2_subs

Subscriber session creation is initiated by an ingress or egress unclassified packet going through
a parent VIF. A packet is considered unclassified when its ip address doesn't match the ip address stored
in any subscriber session. When an unclassified packet is a an ingress packet it means its source ip address doesn't
belong to any session, when an unclassified packet is egress packet its destination address was checked.

To authorized a subscriber session creation RADIUS protocol is used.
Session authorization request includes the following attributes:

todo

## L3 connected subscribers

<img src="http://therouter.net/images/bras/l3_connected_subsc_overview.png">

Subscribers L2 traffic is terminated on a separate router which is connected to the_router via
an interface. That interface would be a parent interface for the subscribers.
A parent interface can use any type of ethernet encapsulation: untagged, dot1q, qinq.
The VIF flag 'l3_subs' should be included in a parent interface creation command.

	vif add name v21 port 0 type dot1q cvid 21 flags l3_subs

Subscriber session creation is initiated by an ingress or egress unclassified packet going through
a parent VIF. A packet is considered unclassified when its ip address doesn't match the ip address stored
in any subscriber session. When an unclassified packet is an ingress packet it means its source ip address doesn't
belong to any session, when an unclassified packet is egress packet its destination address was checked.

To authorized a subscriber session creation RADIUS protocol is used.
Session authorization request includes the following attributes:

todo

## Vlan per subscriber

<img src="http://therouter.net/images/bras/vlan_per_subsc_overview.png">

Each subscriber is connected to a network via a dedicated vlan (dot1q or qinq).
TheRouter is connected to the subscriber network via a port having access to
all subscribers vlans. That port would be a parent port for dynamically created
subscribers virtual interfaces (sessions). Each subscriber will have each own
dynamic VIF. Creation of a dynamic VIF is initiated by an ingress unclassified packet.
A packet is considered unclassified when it doesn't belong to any known dynamic or normal/static VIF.
The port flag 'dynamic_vif' should be included in a parent port creation command.

	port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif

### Authorization of dynamic VIFs

Authorization of dynamic VIFs is using RADIUS protocol.
Dynamic VIF authorization request includes the following attributes:

 * THE_ROUTER_VSA_OUTER_VID - outer vlan id or service vlan id (svid). 0 value is used for dot1q dynamic VIFs
	
 * THE_ROUTER_VSA_INNER_VID - inner vlan id or customer vlan id (cvid)

 * THE_ROUTER_VSA_PORT_ID - TheRouter port number

 * Username - a string containing portid, svid and сvid values.
Those values a concatenated into a string using a dot as a separated field value.
	
### Authorization response

A response may include the attributes described below. Depending on which attributes are in the response
different actions will be taken as the result of response processing.

 * THE_ROUTER_VSA_IP_UNNUMBERED_VIF - this attribute indicates
  that dynamic VIF should be configured according with the ip unnumbered addressing scheme:
  TheRouter will assign an IP address to the dynamic VIF and create an ip route 
  to subscribers ip address by executing the following commands:

		ip addr add <GW_IP>/32 dev <dynamic_vif>
		ip route add <SUB_IP>/32 dev <dynamic_vif> src <GW_IP>

Where:
 * GW_IP - TheRouter's ip address, the same for all subscribers that belongs the same IP network
 * SUB_IP - subscriber ip address
	
Values of GW_IP,SUB_IP fields are defined by radius attributes:
 * THE_ROUTER_VSA_IPV4 or Framed-IP-Address attributes define the SUB_IP value.
 The value of the attribute must be unsigned integer.
 * THE_ROUTER_VSA_IPV4_MASK or Framed-IP-Netmask attributes define the network mask for SUB_IP. 
 The value of the attribute must be unsigned integer within the range 1 - 30.
 * THE_ROUTER_VSA_IPV4_GW attribute defines the GW_IP value.
 The value of the attribute must be unsigned integer.
	
THE_ROUTER_VSA_IPV4_GW attribute is not mandatory. 
If it is not included the first ip address in the network is used as the GW_IP value.
	
If THE_ROUTER_VSA_IP_UNNUMBERED_VIF is not included in the authorization response
then the following commands will be executed to configure a dynamic vif:

		ip addr add <IP>/<MASK> dev <dynamic_vif>
		ip route add <NET>/<MASK> dev <dynamic_vif> src <IP>

Where: IP value is defined by the THE_ROUTER_VSA_IPV4 or Framed-ip-address attribute,
MASK is defined by THE_ROUTER_VSA_IPV4_MASK or Framed-IP-Netmask attributes
and NET value is calculated by masking host bits of the IP value.
	
 * THE_ROUTER_VSA_INGRESS_CIR, THE_ROUTER_VSA_EGRESS_CIR -
these attributes define ingress and egress traffic rate values for traffic shaping mechanism.
Value is a traffic rate limit in kbit/s.

* WISPr-Bandwidth-Max-Down, WISPr-Bandwidth-Max-Up -
these attributes define egress and ingress traffic rate values for traffic shaping mechanism.
Value is a traffic rate limit in bit/s.

 * THE_ROUTER_VSA_PBR attributes controls PBR routing mechanism for a subscriber.
The detailed description of PBR routing mechanism is in the next paragraph.

# Unauthorised subscribers traffic

Unauthorized subscribers traffic can be routed to a separate path.
To do so TheRouter uses Policy-Based Routing (PBR). PBR rules define
which routing table should be used for a particular packet. 

To use this feature the following steps are required:

 * create an additional routing table and install routes into it;
 * create tables for storing unauthorized subscribers identifiers;
 * create PBR rules which will be using unauthorized subscriber identifier tables and the additional routing tables;
 * use PBR Radius attributes in the authorization process or use CoA radius feature to store id of blocked/unauthorized
 user sessions or dynamic VIFs in the special table with ids;

After these steps are completed TheRouter stores unauthorized subscriber ids in the predefined table or
CoA mechanism makes TheRouter store session id in the predefined table.
Then, PBR rules are applied to a receiving packet and if the the_router finds 
a match to one of those rules it will route the packet according to the routes of the routing table specified in the matched PBR rule.

## Additional routing table

	ip route table add <table name>

Example:

	ip route table add blocked_subsc

## PBR rules

### PBR description

PBR mechanism maintains PBR rules table.
Rules are applied to each incoming packet and define which routing table should be used to route the packet.
The rules are applied in the priority order. Once a matching rule is found further rules checking is stopped and
the routing table specified in the found rule is used to route the packet.

### PBR rules for L2/L2 subscribers

Incoming L2/L3 subscriber packets are identified by source ip addresses.
Therefore PBR rules that want to match L2/L3 subscribers packets should use tables
storing ip address. Such PBR rules match if subscribers source ip address is found
in the specified table.

For example, the following command adds PBR rule with priority 10 to the PBR rule list.
Any packet with source ip address found in table 'ips1' will be routed according to the routes
from routing table 'blocked_subsc'

	ip pbr rule add prio 10 u32set ips1 type "ip" table blocked_subsc

### PBR rules for dynamic VIF subscribers

Incoming traffic of dynamic VIF subscribers is identified by the L2_ID value, which consists 
of vlan id values from a packet ethernet header and port number on which the packet was received.
Therefore, PBR rules that want to match dynamic VIF subscribers should use tables storing L2_ID values 
of unauthorised subscribers.

For example, the following command adds PBR rule with priority 20 to the PBR rule list.
Any packet whit L2_ID found in the table 'l2s1' will be routed according to the routes
from routing table 'blocked_subsc'

	ip pbr rule add prio 20 u32set l2s1 type "l2" table blocked_subsc

## Tables storing unauthorized user IDs

### Tables storing IP addresses of unauthorized L2/L3 connected subscribers
An example:

	u32set create ips1 size 4096 bucket_size 16


### Tables stroring L2_IDs of unauthorised dynamic VIF subscribers
An example:

	u32set create l2s1 size 4096 bucket_size 16

## PBR Radius attributes

THE_ROUTER_VSA_PBR attribute configures PBR mechanism for a subscriber.
Depending on the attribute value subscriber id is added or deleted from the table for unauthorized subscribers.
Attribute value 1 is used for additions, 2 for deletions.
Names of the tables used as tables storing unauthorized subscriber id are defined in the configuration file:

	subsc u32set init <IP_TABLE_NAME> <L2_TABLE_NAME>

Where:

 * IP_TABLE_NAME - name of the table storing ip address of anauthorized L2/L3 subscribers.
	
 * L2_TABLE_NAME - name of the table storing L2_ID dynamic VIF subscribers.

!! IMPORTANT NOTE: routing tables, PBR rules, tables storing unauthorized subscriber ids
must be defined in the configuration file before this command.

# RADIUS

## Radis client configuration

	radius_client add server 192.168.3.2
	radius_client add src ip 192.168.3.1
	radius_client set secret "secret"

# Radius/COA

CoA mechanism may be used for
 * changing rate-limiting feature parameters of a subscriber traffic
 * disconnect subscriber sessions
 * unauthorize subscriber to redirection its traffic by PBR mechanism

To accomplish the above tasks the following radius attributes can be used:

 * THE_ROUTER_VSA_PBR
 * THE_ROUTER_VSA_INGRESS_CIR
 * THE_ROUTER_VSA_EGRESS_CIR
 * WISPr-Bandwidth-Max-Down
 * WISPr-Bandwidth-Max-Up

## Alter subscriber shaping values

### Changing rate-limiting feature parameters of a subscriber traffic

Example of changing rate-limiting parameters for dynamic VIF subscribers connected to port 0 via QinQ vlan (svid 10, cvid 20)
Rate-limit values are defined in the VSAs therouter_ingress_cir and therouter_engress_cir.
The following command define ingress rate-limit 101 Mbit/s and egress rate-limit 102 Mbit/s.

	echo User-Name=0:10:20,Vendor-Specific = "TheRouter,therouter_ingress_cir=101000", Vendor-Specific = "TheRouter,therouter_engress_cir=102000" | radclient 192.168.3.1:3799 coa secret	

	!! both THE_ROUTER_VSA_INGRESS_CIR and THE_ROUTER_VSA_EGRESS_CIR must be defined

### Redirecting subscribers traffic

	echo User-Name=0:10:20,User-Password=mypass,Vendor-Specific = "TheRouter,therouter_pbr=1" | radclient 192.168.3.1:3799 coa secret

Where:

therouter_pbr - code defining which PBR action should be taken:

	1 - add
	2 - delete

As a result id of the subscriber with name '0:10:20' will be added to the table of type l2set defined by TheRouter's configuration file.

### Subscriber disconnect

	echo User-Name=0:0:130 | radclient 192.168.3.1:3799 disconnect secret

# DHCP relay

	dhcp_relay 192.168.3.2


# Other commands

### Output subscriber sessions

### Viewing dynamic VIF subscriber ip addresses

	h4 src # rcli sh ip addr
	name    port    vid     encapsulation
	v3      0       0.3     dot1q
	        192.168.3.1/24  primary
	dvif0.0.130       0       0.130   dot1q
	        10.10.0.1/32    h
	v2      0       0.2     dot1q
	        192.168.1.112/24        primary

Where dvif0.0.130 - dynamic VIF (svid 0, cvid 130)

### Output list of routing tables

	h4 src # rcli sh ip route tables
	route table name
	main
	blocked_subsc

### Output PBR rules

	h4 src # rcli sh ip pbr rules
	prio    condition       rt_table
	10      ipset ips1      rt_bl
	20      l2set l2s1      rt_bl

### Output table storing unauthorised subscriber ids

#### Output ip address table

	h4 src # rcli sh ipset ips1
	U32SET name ips1, type IP, hash size 4096, hash bucket size 16, num items: 0
	ipv4 address

#### Output L2_ID tables

	h4 src # rcli sh l2set l2s1
	U32SET name l2s1, type L2, hash size 4096, hash bucket size 16, num items: 0
	portid.svid.cvid


## TheRouter configuration example

#### TheRouter

	startup {
	  sysctl set mbuf 8192
	  port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  rx_queue port 1 queue 2 lcore 1
	  rx_queue port 1 queue 1 lcore 2
	  rx_queue port 1 queue 0 lcore 3
	
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 300
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	  sysctl set vif_stat 1
	  
	  sysctl set pppoe_max_subsc 50000
	  sysctl set radius_max_sessions 20000
	  sysctl set subsc_vif_max 50000
	  	
	  sysctl set dhcp_relay_enabled 1
	  sysctl set system_name "tr1"
	}
	
	runtime {
	
	  ip route add 10.10.0.0/24 unreachable
	
	  # link with local linux host
	  vif add name v3 port 0 type dot1q cvid 3
	  ip addr add 192.168.3.1/24 dev v3
	
	  # home network link
	  vif add name v2 port 0 type dot1q cvid 2 flags npf_on
	  ip addr add 192.168.1.112/24 dev v2
	  ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112
	
	
	  ## static arp records
	  # radius server
	  arp add 192.168.3.2 90:e2:ba:4b:b3:17 dev v3 static
	
	  dhcp_relay 192.168.3.2
	
	  radius_client add server 192.168.3.2
	  radius_client add src ip 192.168.3.1
	  radius_client set secret "secret"
	  coa server set secret "secret"
	
	
	  # PBR
	  ip route table add rt_bl
	
	  u32set create ips1 size 4096 bucket_size 16
	  u32set create l2s1 size 4096 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl
	  ip pbr rule add prio 20 u32set l2s1 type "l2" table rt_bl
	
	
	  # NPF
	  # npf load "/etc/npf.conf.accept_all_2"
	
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}
	
#### NPF /etc/npf.conf.bras_dhcp_relay

	map v2 netmap 10.110.0.0/29
	#alg "icmp"
	
	group default {
	  pass stateful final on v2 all
	  pass final on lh3 all
	}
