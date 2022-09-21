# 1. Introduction

# 3. Network scheme

<img src="http://therouter.net/images/bras/bras vrrp.png">

# 4. Configuration

## 4.1. BNG, virtual router 1

	startup {

	}
	
	runtime {
		# keepalive, db sync interface
		subsc vrrp init dev sv_if
	
		#
		# Subscriber VRRP group 10
		#
		subsc vrrp create group 10 prio 100 neighbor 10.8.0.2
		subsc vrrp group 10 add vif port 1 svid 30 cvid 1
		subsc vrrp group 10 add vif range svid 30 cvid 200 250 name ipoe_
		subsc vrrp group 10 add ip 192.168.2.1
		subsc vrrp group 10 add ip 192.168.3.1
		subsc vrrp group 10 enable
		
		#
		# Subscriber VRRP group 20
		#
		subsc vrrp create group 20 prio 50 neighbor 10.8.0.2
		subsc vrrp group 20 add vif port 1 svid 30 cvid 2
		subsc vrrp group 20 add vif range svid 30 cvid 200 250 name ipoe2_
		subsc vrrp group 20 add ip 192.168.4.1
		subsc vrrp group 20 add ip 192.168.5.1
		subsc vrrp group 20 enable		
	}

## 4.2. BNG, virtual router 2

	startup {

	}
	
	runtime {

	}

