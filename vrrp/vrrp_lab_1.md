# VRRP Lab #1. Testing the VRRP control plane and the vrrp state machine states.

lab#1 test network consists of two VRRP routers: r1 - cisco router (GNS simulator) 
with ip address 192.168.1.91 and h5 - the_router with ip address 192.168.1.111.
Those routers will be participating in one VRRP virtual router (group 10).

## 1) Initial configuration.

	Cisco's config:
	
	interface FastEthernet0/0
	 ip address 192.168.1.91 255.255.255.0
	 duplex auto
	 speed auto
	 vrrp 10 ip 192.168.1.138
	 vrrp 10 ip 192.168.1.139 secondary
	 vrrp 10 priority 100
	!

TheRouter's config

	...
	
	runtime {
	  ip addr add 5.5.5.5/32 dev lo
	  ip route add 224.0.0.0/4 unreachable
	  ip route add 10.10.0.0/24 unreachable
	  
	  vif add name v3 port 0 type dot1q cvid 3
	  ip addr add 192.168.1.111/24 dev v3
	  ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.111
	  
	  #
	  # vrrp group 10
	  #
	  vrrp create group 10 dev v3
	  vrrp group 10 dev v3 prio 150
	  vrrp group 10 dev v3 ip add 192.168.1.138
	  vrrp group 10 dev v3 ip add 192.168.1.139 secondary
	}

Router's log:

	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: Loading configuration ...
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: Loading the runtime part of configuration file '/etc/router_vrrp.conf'...
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: add lba_addr 3232236031 for 3232235887/24
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: add lba_addr 3232236799 for 3232236545/24
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: lcore 0: vrrp added group 10 virtual mac address 00:00:5E:00:01:0A on port 0
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: lcore 0: vrrp group 10 is created on vif v3
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: lcore 0: vrrp group 10 on vif v3 changed state, init -> backup
	Apr 20 21:08:32 h5 the_router[31046]: ROUTER: router configuration file '/etc/router_vrrp.conf' successfully loaded
	Apr 20 21:08:36 h5 the_router[31046]: ROUTER: lcore 0: vrrp group 10 on vif v3 changed state, backup -> master

Cisco VRRP group state

	R1#sh vrrp
	FastEthernet0/0 - Group 10
	  State is Backup
	  Virtual IP address is 192.168.1.138
	    Secondary Virtual IP address is 192.168.1.139
	  Virtual MAC address is 0000.5e00.010a
	  Advertisement interval is 1.000 sec
	  Preemption enabled
	  Priority is 100
	  Master Router is 192.168.1.111, priority is 150
	  Master Advertisement interval is 1.000 sec
	  Master Down interval is 3.609 sec (expires in 3.417 sec)


TheRouter VRRP group state

	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is master
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 150
	  master router is 192.168.1.111 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

## 2) Changing priority.

Changing priority to a value less then other VRRP router priority value
should trigger a new master election.

	rcli vrrp group 10 dev v3 prio 50

TheRouter has transitioned to a backup state and cisco became a master router.

	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is backup
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 50
	  master router is 192.168.1.91, priority is 100
	  master router advertisement interval is 1 sec
	  master down interval is 3.804 sec

Cisco

	R1#sh vrrp
	FastEthernet0/0 - Group 10
	  State is Master
	  Virtual IP address is 192.168.1.138
	    Secondary Virtual IP address is 192.168.1.139
	  Virtual MAC address is 0000.5e00.010a
	  Advertisement interval is 1.000 sec
	  Preemption enabled
	  Priority is 100
	  Master Router is 192.168.1.91 (local), priority is 100
	  Master Advertisement interval is 1.000 sec
	  Master Down interval is 3.609 sec

## 3) Enabling/disabling

Disable the_router vrrp group 10 on interface v3.

	h5 ~ # rcli vrrp group 10 dev v3 disable
	h5 ~ #
	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is init
	    administratively shutdown
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 150
	  master router is 0.0.0.0, priority is 0
	  master router advertisement interval is 0 sec
	  master down interval is 3.414 sec

Enable the_router vrrp group 10 on interface v3.

	h5 ~ # rcli vrrp group 10 dev v3 enable

Router has taken a master role back.

	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is master
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 150
	  master router is 192.168.1.111 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

## 4) Preemption mode.

Don't allow a router to become master even if it's priority is higher then others VRRP routers.

	h5 ~ # rcli vrrp group 10 dev v3 preempt off
	h5 ~ #
	h5 ~ # rcli vrrp group 10 dev v3 disable
	h5 ~ #
	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is init
	    administratively shutdown
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption off
	  priority 150
	  master router is 0.0.0.0, priority is 0
	  master router advertisement interval is 0 sec
	  master down interval is 3.414 sec

Now enable the_router back online and make sure it's not a master vrrp router.

	h5 ~ # rcli vrrp group 10 dev v3 enable
	h5 ~ #
	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is backup
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption off
	  priority 150
	  master router is 192.168.1.91, priority is 100
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

Turn on preemtion mode. It will allow the_router to become a master.

	h5 ~ # rcli vrrp group 10 dev v3 preempt on
	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is master
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 150
	  master router is 192.168.1.111 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

# 5) Router link failure.

Deletion a virtual link in the GNS project will create a split brain situation.
And both routers become a master router.

Cisco

	R1#sh vrrp
	FastEthernet0/0 - Group 10
	  State is Master
	  Virtual IP address is 192.168.1.138
	    Secondary Virtual IP address is 192.168.1.139
	  Virtual MAC address is 0000.5e00.010a
	  Advertisement interval is 1.000 sec
	  Preemption enabled
	  Priority is 100
	  Master Router is 192.168.1.91 (local), priority is 100
	  Master Advertisement interval is 1.000 sec
	  Master Down interval is 3.609 sec


TheRouter

	h5 ~ # rcli sh vrrp
	vif v3 (port 0, vid 0.3, type 0) - group 10
	  state is master
	  primary ip address 192.168.1.138
	    secondary ip address 192.168.1.139
	  advertisement interval 1 sec
	  preemption on
	  priority 150
	  master router is 192.168.1.111 (this system), priority is 150
	  master router advertisement interval is 1 sec
	  master down interval is 3.414 sec

