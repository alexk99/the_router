# Installing 

Here is the installation process description for Gentoo Linux.

## Install the following utilities and libs:

 * git

		emerge git

 * rpm

		emerge rpm

 * svn

		emerge subversion

 * libpcap
 
		emerge libpcap

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

 * Clone NPF

		git clone https://github.com/alexk99/npf

 * Download <a href="http://therouter.net/downloads/city/city.h">city.h</a>, then

		cp city.h /urs/local/include/

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
	- If you use grub edit /boot/grub/grub.conf and appent the following options:
	
			intel_idle.max_cstate=1 isolcpus=1,2,3 default_hugepagesz=2M hugepagesz=2M hugepages=3072

	- Note:
		You might want to isolate a different set of cores or reserve different amount of ram for huge pagas 
		depending of the hardware configuration of your server.
		The rule here is that you should isolate all cores you're going to use in the router's traffic forwarding process unless
		the perfomance is not the goal.
		
* download dpdk 16.07

		wget http://fast.dpdk.org/rel/dpdk-16.07.tar.xz
		tar xvf dpdk-16.07.tar.xz
		cd ./dpdk-16.07
		
* Download dpdk log subsystem patch <a href="http://therouter.net/downloads/dpdk/log_patch.patch">log_patch.patch</a>, then
  apply the patch
  		
		cat ./log_patch.patch | patch -p2
		
* Run the following commands:		

		make install T=x86_64-native-linuxapp-gcc


## Install the router

 
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

### Install the router 

 * Download the router <a href="http://therouter.net/downloads/the_router.a0.01.tar.gz">the_router.a0.01.tar.gz</a>

 * Run the following commands:
 
 		tar xvf ./the_router.a0.01.tar.gz
 		cd ./the_router.a0.01
 		./install.sh

### Configure dpdk ports

* Define $RTE_SDK variable

		export RTE_SDK=/path_to_dpdk

* Load drivers

		modprobe uio
		insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
		# loading kni module 
		insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/rte_kni.ko

* Bind your NIC's to DPDK either by using the commands below or by 
  running the $RTE_SDK/tools/setup.sh. If you are going to run 
  the following commands make sure you are using your own NIC PCI
  addresses in the echo commands

	- Down linux interfaces of NIC you are goiing to bind to dpdk, for example:
	
			ip link set down enp1s0f0
			ip link set down enp1s0f0

	- Unbind NIC's from linux
	
			# replace 0000:01:00.0 with your own address.
			# use lspci to determine it.
			echo 0000:01:00.0 > /sys/bus/pci/drivers/ixgbe/unbind
			echo 0000:01:00.1 > /sys/bus/pci/drivers/ixgbe/unbind

	- Bind NIC's to the DPDK driver
	
			# replace "8086 10fb" with your own addres
			# echo vendor device (lspci -n)
			echo "8086 10fb" > /sys/bus/pci/drivers/igb_uio/new_id

### Run the router

 * Prepare configuration files. For configuration examples and options see the page <a href="/conf_options.md">Configure the Router</a>

 	- create router.conf

			nano /etc/router.conf

 	- create npf conf

			nano /etc/npf.conf

 * run the router

		/usr/local/sbin/router_run.sh /etc/the_router.conf

