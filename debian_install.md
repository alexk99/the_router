# Install

A 30-day trial build of BisonRouter is available only by request.
Please contact us by email info at bisonrouter.com

Here are installation steps for Ubuntu 20.10.

## Install DPDK

### Configure linux kernel

* Turn on linux boot time options:
	- Edit GRUB_CMDLINE_LINUX variable in the /etc/default/grub

			GRUB_CMDLINE_LINUX="intel_idle.max_cstate=1 isolcpus=1,2,3,4,5,6,7,9,10,11,12,13,14,15 default_hugepagesz=2M hugepagesz=2M hugepages=3072"

	- Run

			update-grub

	- Note:
		You might want to isolate a different set of cores or reserve different amount of ram for huge pages 
		depending of the hardware configuration of your server.
		The rule here is that you should isolate all cores you're going to use in the router's traffic forwarding process unless
		the perfomance is not a goal.

* Configure hugepages

	- reboot you machine and check that hugepages are available and free

			grep -i huge /proc/meminfo

	- you should get something like this:

			HugePages_Total:    3072
			HugePages_Free:     3072
			HugePages_Rsvd:        0
			HugePages_Surp:        0
			Hugepagesize:       2048 kB

	- Make a mount point for hugepages

			mkdir /mnt/huge

	- Create a mount point entry in the /etc/fstab

			huge         /mnt/huge   hugetlbfs pagesize=2M   0       0

	- Mount hugepages

			mount huge

### Patch and install DPDK

Download DPDK 18.11.11

		cd /usr/src/
		wget http://fast.dpdk.org/rel/dpdk-18.11.11.tar.xz
		tar xvf dpdk-18.11.11.tar.xz
		cd ./dpdk-stable-18.11.11

Run the following commands:		

		make install T=x86_64-native-linuxapp-gcc

## BisonRouter

### Download BisonRouter 

 Please, contact us at info@bisonrouter.net
 
### Install BisonRouter

		apt install ./bison-router_xxx.deb

### Configure DPDK ports

#### Define RTE_SDK variable

		export RTE_SDK=/usr/src/dpdk-stable-18.11.11

 Add the above export command to /root/.profile
  
  		nano /root/.profile

#### Load drivers and bind your NICs to DPDK

Edit /usr/sbin/load_dpdk_drivers.sh script
and replace 0000:xx:xx.x values with PCI addresses of your NICs, for examaple 0000:02:00.0.
Run 

		$RTE_SDK/usertools/dpdk-devbind.py --status

to find out NIC's PCI addresses.

After correct PCI addresses of your NIC are added to the load_dpdk_drivers.sh file,
run it and then check that NICs are switched to DPDK mode by running again

		$RTE_SDK/usertools/dpdk-devbind.py --status

This time you should find your NICs in the "Network devices using DPDK-compatible driver" section.

### Run BisonRouter

#### Prepare configuration files

For configuration examples and options see the page <a href="/conf_options2.md#therouter-configuration-file-example">Command reference</a>

 - create router.conf

		nano /etc/router.conf

#### Run BisonRouter

  Define a bash alias

  		alias rcli="ip netns exec tr rcli"

  Add the alias to /root/.profile so it's created everytime you log in.
  
  		nano /root/.profile

  Edit /usr/sbin/therouter_start.sh script
  and use your NIC's addresses as values for -w parameters. If you have only
  one NIC then delete the second -w parameter.

  Start BisonRouter

		therouter_start.sh /etc/router.conf

  Check the syslog to ensure that BisonRouter has started successfully.

 		ROUTER: router configuration file '/etc/router.conf' successfully loaded  

  Use the 'rcli' alias which will execute an rcli utility to configure and control BisonRouter

		# rcli sh uptime
		Uptime: 0 day(s), 1 hour(s), 38 minute(s), 14 sec(s)
  
