# Install

Here is the installation for Ubuntu 16.04.

## Install the following utilities and libs:

	apt install g++
	apt install libjemalloc-dev
	apt install libpcap-dev
	apt install python
	apt install libpcre2-8-0
	apt install autoconf
	apt install zlib1g-dev
	apt install flex
	apt install byacc
	apt install cmake
	apt install libtool
	apt install libtool-bin
	apt install subversion
	apt install rpm
	apt install libreadline6 libreadline6-dev

## Install libcap

	wget http://www.tcpdump.org/release/libpcap-1.5.3.tar.gz
	tar xzvf ./libpcap-1.5.3.tar.gz
	cd ./libpcap-1.5.3/
	./configure
	make
	make install

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

Notes: install process ends successfully even if it indicates that there has been the following error:

	make -C man install
	make[1]: Entering directory '/home/alex/libcdb/man'
	make[1]: *** No rule to make target 'install'.  Stop.
	make[1]: Leaving directory '/home/alex/libcdb/man'
	Makefile:5: recipe for target 'install' failed
	make: *** [install] Error 2	

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

		mkdir /usr/lib64
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

 * Run the following commands:
 
		cd npf/src
		
		cd libnpf/net
		rm ./npf.h
		ln -s ../../kern/npf.h npf.h		
		cd ..
		
		export DESTDIR=/
		export LIBDIR=/usr/lib64
		export INCDIR=/usr/local/include
		export MANDIR=/usr/local
		make
		make install

## Install DPDK

### Configure linux kernel

* Make shure the following options are enabled:

		UIO support
		   Device Drivers -> Userspace I/O drivers -> Generic driver for PCI 2.3 and PCI Express cards
		      symbol UIO_PCI_GENERIC
		
		PROC_PAGE_MONITOR
		   File systems -> Pseudo filesystems -> /proc file system support      
		
		HUGETLBFS
		  File systems -> Pseudo filesystems
		
		HPET and HPET_MMAP
		   Device Drivers -> Character devices -> HPET - High Precision Event Timer

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

* Turn on linux boot time options:
	- Edit GRUB_CMDLINE_LINUX variable in the /etc/default/grub
	
			GRUB_CMDLINE_LINUX="intel_idle.max_cstate=1 isolcpus=1,2,3,4,5,6,7,9,10,11,12,13,14,15 default_hugepagesz=2M hugepagesz=2M hugepages=3072"

	- Note:
		You might want to isolate a different set of cores or reserve different amount of ram for huge pages 
		depending of the hardware configuration of your server.
		The rule here is that you should isolate all cores you're going to use in the router's traffic forwarding process unless
		the perfomance is not a goal.
		
* download dpdk 16.07

		wget http://fast.dpdk.org/rel/dpdk-16.07.tar.xz
		tar xvf dpdk-16.07.tar.xz
		cd ./dpdk-16.07
		
### Patch DPDK

Download the patches:

 * <a href="http://therouter.net/downloads/dpdk/patches/16.07/log_patch_dpdk_16.07.patch">dpdk log subsystem patch</a>
 * <a href="http://therouter.net/downloads/dpdk/patches/16.07/net_bond_mempool_fix_16.07.patch">net bond mempool patch</a>

Apply the patches:

		cat ./log_patch_dpdk_16.07.patch | patch -p2
		cat ./net_bond_mempool_fix_16.07.patch | patch -p2

Run the following commands:		

		make install T=x86_64-native-linuxapp-gcc


## Install TheRouter

 
### Install dependencies

 * Download quagga sources <a href="http://therouter.net/downloads/quagga-1.0.20160315.tar.xz">quagga-1.0.20160315.tar.xz</a>
	
		tar xvf ./quagga-1.0.20160315.tar.xz

 * city hash

		git clone https://github.com/google/cityhash
		cd ./cityhash/
		./configure
		make all check CXXFLAGS="-g -O3"
		make install

 Notes: libcityhash should be compiled as dynamic library, but the steps below create a static link lib.
 I haven't so far figured why, you can download compiled lib <a href="http://therouter.net/downloads/libcityhash.so.0.0.0">libcityhash.so.0.0.0</a>
 and copy it to /usr/local/lib/libcityhash.so.0.0.0

### Install TheRouter 

 !  Note: this version supports maximum 6 cores cpu. 

 * Download TheRouter <a href="http://therouter.net/downloads/the_router.a0.13.6cores.dpdk.16.07.tar.gz">the_router.a0.13.6cores.dpdk.16.07.tar.gz</a>
 * Download TheRouter (the version for old xeon cpu: X5355, X5650, etc..)<a href="http://therouter.net/downloads/the_router.a0.12.6cores.old_xeon.dpdk.16.07.tar.gz">the_router.a0.12.6cores.old_xeon.dpdk.16.07.tar.gz</a> 

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

* Bind your NIC's to DPDK either by using $RTE_SDK/tools/dpdk-devbind.py

		$RTE_SDK/tools/dpdk-devbind.py --bind=igb_uio 0000:02:00.0
		$RTE_SDK/tools/dpdk-devbind.py --bind=igb_uio 0000:02:00.1

### Run TheRouter

 * Prepare configuration files. For configuration examples and options see the page <a href="/conf_options.md">Configure TheRouter</a>

 	- create router.conf

			nano /etc/router.conf

 	- create npf conf

			nano /etc/npf.conf

 * run the router

		/usr/local/sbin/router_run.sh /etc/router.conf

