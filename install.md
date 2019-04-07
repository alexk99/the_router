# Install

Here are the installations steps for Gentoo Linux.

## Install the following utilities and libs:

		emerge git
		emerge rpm
		emerge subversion
		emerge libpcap
		emerge sys-process/numactl

## Install Proplib

Download <a href="http://therouter.net/downloads/proplib-0.6.3.tar.xz">proplib-0.6.3.tar.xz</a>

		tar xvf ./proplib-0.6.3.tar.xz
		cd ./proplib-0.6.3
		./configure
		make
		make install

## Install libcdb
 
 * Download <a href="http://therouter.net/downloads/libcdb.tar.gz">libcdb.tar.gz</a>
   or

		git clone https://github.com/rmind/libcdb

 * Download the patch <a href="http://therouter.net/downloads/libcdb_alexk.patch">libcdb_alexk.patch</a>
 * Execute the following commands:
 

		cd ./libcdb
		cat libcdb_alexk.patch | patch -p1

		export LIBDIR=lib
		export INCDIR=include
		export DESTDIR=/usr/local
		make all
		make install

## Install qsbr

 * Download <a href="http://therouter.net/downloads/libqsbr.tar.gz">libqsbr.tar.gz</a>
 * Download the patch <a href="http://therouter.net/downloads/libqsbr_alexk.patch">libqsbr_alexk.patch</a>
 * Execute the following commands:

		tar xzvf libqsbr.tar.gz
		cd ./libqsbr/src
		cat libqsbr_alexk.patch | patch -p1

		export LIBDIR=lib
		export INCDIR=include/qsbr
		export DESTDIR=/usr/local
		make all
		make install


## Install bpfjit

 * Download <a href="http://therouter.net/downloads/bpfjit.tar.gz">bpfjit.tar.gz</a>
 * Download <a href="http://therouter.net/downloads/sljit-0.92.tgz">sljit-0.92.tgz</a>
 * Execute the following commands:

		tar xzvf ./bpfjit.tar.gz
		tar xzvf ./sljit-0.92.tgz
		cd ./bpfjit/sljit/
		cp -rpn ../../sljit-0.92/* ./
		cd ..

 * Edit ./SPECS/libbpfjit.spec and delete or comment the following lines:
 
		BuildRequires:>make
		BuildRequires:>libtool

 * Run

		make rpm
		rpm --nodeps -ihv RPMS/x86_64/libbpfjit-0.1-1.x86_64.rpm


## Install NPF

 * Clone NPF (https://github.com/alexk99/npf/tree/alexk)

		git clone -b alexk https://github.com/alexk99/npf

 * Download <a href="http://therouter.net/downloads/city/city.h">city.h</a>, then

		cp city.h /usr/local/include/

 * Install jemalloc

		emerge jemalloc

 * Run the following commands:
 
		cd npf/src
		export LIBDIR=/usr/lib64
		export INCDIR=/usr/local/include
		export MANDIR=/usr/local
		make
		make install

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
	- If you use grub edit /boot/grub/grub.conf and appent the following options:
	
			intel_idle.max_cstate=1 isolcpus=1,2,3 default_hugepagesz=2M hugepagesz=2M hugepages=3072

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
		
* download dpdk 17.11.1 (LTS)

		wget https://fast.dpdk.org/rel/dpdk-17.11.1.tar.xz
		tar xvf dpdk-17.11.1.tar.xz
		cd ./dpdk-stable-17.11.1
		
### Patch DPDK

Download the patches:

 * <a href="http://therouter.net/downloads/dpdk/patches/17.11.1/eal_log.patch">eal log patch</a>
 * <a href="http://therouter.net/downloads/dpdk/patches/17.11.1/bond_fix_mtu.patch">net bond fix mtu patch</a>
 * <a href="http://therouter.net/downloads/dpdk/patches/17.11.1/bond_lacp_fix_mempool_size.patch">net bond mempool patch</a>
 * <a href="http://therouter.net/downloads/dpdk/patches/17.11.1/ip_fragmentation_table.patch">ip fragmentation table patch</a>
 * <a href="http://therouter.net/downloads/dpdk/patches/17.11.1/bond_add_remove_mac_addr.patch">bond_add_remove_mac_addr patch</a>

Apply the patches:

		cat ./eal_log.patch | patch -p1
		cat ./bond_fix_mtu.patch | patch -p1
		cat ./bond_lacp_fix_mempool_size.patch | patch -p1
		cat ./ip_fragmentation_table.patch | patch -p1
		cat ./bond_add_remove_mac_addr.patch | patch -p1

Run the following commands:		

		make install T=x86_64-native-linuxapp-gcc


## Install TheRouter

 
### Install dependencies

 * Quagga

		emerge -v quagga

 * Download quagga sources <a href="http://therouter.net/downloads/quagga-1.0.20160315.tar.xz">quagga-1.0.20160315.tar.xz</a>
	
		tar xvf ./quagga-1.0.20160315.tar.xz

 * PCRE2

		emerge libpcre2
 
 * libcuckoo

		git clone https://github.com/efficient/libcuckoo
		cd libcuckoo
		autoreconf -fis
		./configure
		make
		make install

### Download TheRouter 

 #### x86_64

* Download <a href="http://therouter.net/downloads/the_router.latest.6cores.dpdk.17.11.1.b0.50.tar.gz">the_router.6cores.dpdk.17.11.1.dev.b0.49.tar.gz</a>

### Install TheRouter 

 * Run the following commands:
 
 		tar xvf ./the_router.xxx.tar.gz
 		cd ./the_router.xxx
 		./install.sh

### Configure dpdk ports

* Define $RTE_SDK variable

		export RTE_SDK=/path_to_dpdk

* Load drivers

		modprobe uio
		insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
		# loading kni module 
		insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/rte_kni.ko

* Bind your NIC's to DPDK by using $RTE_SDK/usertools/dpdk-devbind.py

		$RTE_SDK/usertools/dpdk-devbind.py --bind=igb_uio 0000:02:00.0
		$RTE_SDK/usertools/dpdk-devbind.py --bind=igb_uio 0000:02:00.1

### Run TheRouter

 * Prepare configuration files. For configuration examples and options see the page <a href="/conf_options.md">Configure TheRouter</a>

 	- create router.conf

			nano /etc/router.conf

 	- create npf conf

			nano /etc/npf.conf

 * run the router

		/usr/local/sbin/router_run.sh /etc/router.conf

