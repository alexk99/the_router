# Install

Latest 30 day trial build of TheRouter is available only by request.
Please contact alex at therouter.net.

Here are installation steps for Ubuntu 18.04.

## Install the following utilities and libs:

		apt update
		apt -y install g++ libjemalloc-dev libpcap-dev python libpcre2-8-0 autoconf zlib1g-dev flex byacc
		apt -y install cmake libtool libtool-bin subversion rpm libreadline-dev libnuma-dev libnl-genl-3-dev


## Install qsbr

		wget http://therouter.net/downloads/libqsbr.tar.gz
		tar xzvf libqsbr.tar.gz
		cd ./libqsbr/src
		wget http://therouter.net/downloads/libqsbr_alexk.patch
		cat libqsbr_alexk.patch | patch -p1
		export LIBDIR=lib
		export INCDIR=include/qsbr
		export DESTDIR=/usr/local
		make all
		make install
		cd ../..

## Install city hash

		git clone https://github.com/google/cityhash
		cd ./cityhash/
		./configure
		make all check CXXFLAGS="-g -O3"
		make install
		cd ..

## Update system library paths

 Add the following lines to the /etc/ld.so.conf.d/router.conf

		/usr/lib64
		/usr/local/lib
		/usr/lib/x86_64-linux-gnu

 Run

		ldconfig

## Install DPDK

### Configure linux kernel

* Make sure the following options are enabled:

		UIO support
		   Device Drivers -> Userspace I/O drivers -> Generic driver for PCI 2.3 and PCI Express cards
		      symbol UIO_PCI_GENERIC
		
		PROC_PAGE_MONITOR
		   File systems -> Pseudo filesystems -> /proc file system support      
		
		HUGETLBFS
		  File systems -> Pseudo filesystems
		
		HPET and HPET_MMAP
		   Device Drivers -> Character devices -> HPET - High Precision Event Timer

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

Download DPDK 18.11.3

		wget https://fast.dpdk.org/rel/dpdk-18.11.3.tar.xz
		tar xvf dpdk-18.11.3.tar.xz
		cd ./dpdk-stable-18.11.3

Download and apply the patches:

		wget http://therouter.net/downloads/dpdk/patches/18.11.3/bonding_mempool.patch
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/bonding_rx_capabilities.patch
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/librte_ip_frag_add_mbuf_counter.patch
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/rte_timer.patch
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/bonding_tx_capabilities.patch
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/bond_lacp.diff
		wget http://therouter.net/downloads/dpdk/patches/18.11.3/scheduler_fix_port_time_rounding.diff
		
		cat ./bonding_mempool.patch | patch -p1
		cat ./bonding_rx_capabilities.patch | patch -p1
		cat ./librte_ip_frag_add_mbuf_counter.patch | patch -p1
		cat ./rte_timer.patch | patch -p1
		cat ./bonding_tx_capabilities.patch | patch -p1
		cat ./bond_lacp.diff | patch -p1
		cat ./scheduler_fix_port_time_rounding.diff | patch -p1

Run the following commands:		

		make install T=x86_64-native-linuxapp-gcc

## TheRouter

### Download TheRouter 

 Please, contact alex@therouter.net

### Configure DPDK ports

#### Define RTE_SDK variable

		export RTE_SDK=/path_to_dpdk

#### Load drivers and bind your NICs to DPDK

Edit load_dpdk_drivers.sh script from TheRouter archive's sbin directory
and replace 0000:xx:xx.x values with PCI addresses of your NICs, for examaple 0000:02:00.0.
Use the 'lspci' utility to find out NIC's addresses.

### Run TheRouter

#### Prepare configuration files

For configuration examples and options see the page <a href="/conf_options.md">Configure TheRouter</a>

 - create router.conf

		nano /etc/router.conf

 - create NPF conf

		nano /etc/npf.conf

#### Run TheRouter

  Define a bash variable 'rvrf'

  		export rvrf="ip netns exec tr"

  Edit therouter_start.sh script located in TheRouter's archive sbin directory
  and use your NIC's addresses as values for -w parameters. If you have only
  one NIC then delete the second -w parameter.

  Start TheRouter

		therouter_start.sh /etc/router.conf

  Check the syslog to ensure that TheRouter has started successfully.

 		ROUTER: router configuration file '/etc/router.conf' successfully loaded  

  Use the 'rcli' utility from the archive to configure and control TheRouter

		# $rvrf rcli sh uptime
		Uptime: 0 day(s), 1 hour(s), 38 minute(s), 14 sec(s)
