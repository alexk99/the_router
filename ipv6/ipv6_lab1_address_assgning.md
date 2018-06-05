# IPV6 Lab #1. Testing the IPV6 address assigning commands

lab#1 test network consists of the_router and a host supporting ipv6

## 1. Enable ipv6 on the_router on interface v3

	rcli ipv6 enable dev v3; rcli sh ipv6 addr

Command enables ipv6 on v3. It generates link-local ipv6 address using EUI-64 algorithm
and starts duplicate address detection (DAD) procedure to ensure the link-local address
is not in use by another node. While DAD procedure is running the address being checked stays
in TENTATIVE mode - flags TEN

	h5 ~ # rcli ipv6 enable dev v3; rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is disabled, link-local address is fe80::6a05:caff:fe30:6180, flags TEN
	  Global unicast address(es):
	  Solicited-node mcast address(es):
	    30:61:80

DAD is over. The link-local address now is a preffered address.

	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	  Solicited-node mcast address(es):
	    30:61:80


## 2. Assigning a global address

	rcli ipv6 addr add 2001:470::24e6:3555:bcec:4100/64 dev v3; rcli sh ipv6 addr

Assign global ip6 address 2001:470::24e6:3555:bcec:4100 on interface v3.

	h5 ~ # rcli ipv6 addr add 2001:470::24e6:3555:bcec:4100/64 dev v3; rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	    2001:470::24e6:3555:bcec:4100, flags TEN
	  Solicited-node mcast address(es):
	    30:61:80
	    ec:41:0

DAD for 2001:470::24e6:3555:bcec:4100 is over. The address now is a preffered address
has an infinite lifetime.

	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	    2001:470::24e6:3555:bcec:4100, flags PREF
	  Solicited-node mcast address(es):
	    30:61:80
	    ec:41:0

## 3. Assigning duplicate link-local address.

Let's manually assign to v3 the link-local address belonging to another node.

	rcli ipv6 addr link-local fe80::24e6:3555:bcec:4a36 dev v3

DAD failed. The link-local address state becomes DUPLICATE - flags DUP.
The ipv6 interface v3 state becomes STALLED. All global address states become TEN.

	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is stalled, link-local address is fe80::24e6:3555:bcec:4a36, flags DUP
	  Global unicast address(es):
	    2001:470::24e6:3555:bcec:4100, flags TEN
	  Solicited-node mcast address(es):
	    30:61:80
	    ec:41:0
	    ec:4a:36

## 4. Assign EUI-64 link-local address 

	rcli ipv6 addr link-local fe80::24e6:3555:bcec:4a36 dev v3

The command above gerenerates a link-local address using EUI-64 procedure, assigns it the
interface v3 and starts DAD procedure for it. Once DAD for link-local address is completed
and address has been proven to be unique the_router start DAD procedure for all global TENTATIVE
addresses assigned to the same interface.
	
All DAD procedures are completed. All addresses are preffered (flags PREF)
	
	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	    2001:470::24e6:3555:bcec:4100, flags PREF
	  Solicited-node mcast address(es):
	    ec:4a:36
	    ec:41:0
	    30:61:80

## 5. Assign a global address using a prefix and EUI-64 interface id

	rcli ipv6 addr add 2001:470::0/64 eui-64 dev v3

Command generates a global address using the given prefix and the interface eui-64 identifier

	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	    2001:470::6a05:caff:fe30:6180, flags PREF
	    2001:470::24e6:3555:bcec:4100, flags PREF
	  Solicited-node mcast address(es):
	    30:61:80
	    ec:41:0

## 6. Delete a global address

	rcli ipv6 addr del 2001:470::24e6:3555:bcec:4100/64 dev v3
	
The 2001:470::24e6:3555:bcec:4100 address has been deleted from the v3 interface

	h5 ~ # rcli sh ipv6 addr
	vif v3
	  port 0, vlan 0.3, encapsulation dot1q
	  IPv6 is enabled, link-local address is fe80::6a05:caff:fe30:6180, flags PREF
	  Global unicast address(es):
	    2001:470::6a05:caff:fe30:6180, flags PREF
	  Solicited-node mcast address(es):
	    30:61:80
	h5 ~ #
