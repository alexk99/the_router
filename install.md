# Installing 

Here is installation process on Gentoo Linux.

## Install various linux utils

 * git

		emerge git

 * rpm

		emerge rpm

 * svn

		emerge subversion

 * libpcap
 
		emerge libpcap

## Install Proplib

Download <a href='http://arouter.com/downloads/proplib-0.6.3.tar.xz'>proplib-0.6.3.tar.xz</a>

		tar xvf ./proplib-0.6.3.tar.xz
		cd ./proplib-0.6.3
		./configure
		make
		make install

/*
 *	Install libcdb
 */
git clone https://github.com/rmind/libcdb
or
get from here link to libcdb.tar.gz
download from D:\Работа\work\svn\ISP\the_router\npf\deps
libcdb_alexk_path лежит в D:\Работа\work\svn\ISP\the_router\npf\deps

cd ./libcdb
cat ../libcdb_alexk_path | patch -p1

export LIBDIR=lib
export INCDIR=include
export DESTDIR=/usr/local
make all
make install

/*
 *	Install qsbr
 */
git clone https://github.com/rmind/libqsbr
or
download from D:\Работа\work\svn\ISP\the_router\npf\deps

cd ./libqsbr/src
cat ../../libqsbr_alexk_patch | patch -p1

export LIBDIR=lib
export INCDIR=include/qsbr
export DESTDIR=/usr/local
make all
make install


/*
 *	Install bpfjit
 */
download sljit-0.92.tgz from D:\work\npf_deps 
tar xzvf ./sljit-0.92.tgz

git clone https://github.com/rmind/bpfjit
or 
download bpfjit.tar.gz from npf_deps 

cd ./bpfjit/sljit/
cp -rpn ../../sljit-0.92/* ./
cd ./bpfjit/

edit ./Makefile

delete or comment two following lines:
   BuildRequires:>make
   BuildRequires:>libtool

make rpm
rpm --nodeps -ihv RPMS/x86_64/libbpfjit-0.1-1.x86_64.rpm


/*
 *	Install NPF
 */
download npf_github
copy city/city.h to /usr/local/include

emerge jemalloc
 
cd ./npf_github/src
export LIBDIR=/usr/lib64
export INCDIR=/usr/local/include
export MANDIR=/usr/local
make
make install

/*
 * DPDK
 */

/* Prepare linux kernel.
Make shure the following options are enabled:
*/

UIO support
   Device Drivers -> Userspace I/O drivers -> Generic driver for PCI 2.3 and PCI Express cards
      symbol UIO_PCI_GENERIC

PROC_PAGE_MONITOR
   File systems -> Pseudo filesystems -> /proc file system support      

HUGETLBFS
  File systems -> Pseudo filesystems

HPET and HPET_MMAP
   Device Drivers -> Character devices -> HPET - High Precision Event Timer


/* reboot you machine and check that hugepages are free */

	grep -i huge /proc/meminfo

you should get something like this:

	HugePages_Total:    3072
	HugePages_Free:     3072
	HugePages_Rsvd:        0
	HugePages_Surp:        0
	Hugepagesize:       2048 kB

/* mount huge pages */

	mkdir /mnt/huge

create mout poing in the /etc/fstab
	huge         /mnt/huge   hugetlbfs pagesize=2M   0       0
	
mount it
	mount huge

/* Turn on some boot time options */
If you use grub edit /boot/grub/grub.conf and appent following options:

intel_idle.max_cstate=1 isolcpus=1,2,3 default_hugepagesz=2M hugepagesz=2M hugepages=3072

/* download dpdk 16.07 */
http://fast.dpdk.org/rel/dpdk-16.07.tar.xz
tar xvf ./dpdk-16.07.tar.xz
cd ./dpdk-16.07
make install T=x86_64-native-linuxapp-gcc

patch it
todo

/*
 *	Install The_router
 */
 
/* Install dependencies */

/* Quagga */
	emerge -v quagga

/* quagga sources */
	quagga-1.0.20160315.tar.xz
	tar xvf ./quagga-1.0.20160315.tar.xz

/* PCRE2 */
	emerge libpcre2
 
/* libcuckoo */
	git clone https://github.com/efficient/libcuckoo
	or download 
	todo make libcackoo archive 
	
	cd ./libcuckoo
	autoreconf -fis
	./configure
	make
	make install

 
cd ./the_router_alexk_dpdk_v16_07/src/

/* define */
export NPF_BASE_DIR=/usr/src/npf_github
export THE_ROUTER_BASE_DIR=/usr/src/the_router_alexk_dpdk_v16_07
export RTE_SDK=/usr/src/dpdk-16.07
export RTE_TARGET=x86_64-native-linuxapp-gcc
# quagga sources
export QUAGGA_DIR=/usr/src/quagga-1.0.20160315/

make
cp ./build/the_router /usr/local/bin

/* cli */
cd ./cli_client
make
cp ./cli /usr/local/bin/rcli


/* Configure dpdk ports */

/* load drivers */
modprobe uio
insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
# loading kni module 
insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/rte_kni.ko

/* 
bind your NIC to DPDK either by using the commands below or by 
running the $RTE_SDK/tools/setup.sh. If you are going to run 
the following commands make sure you are using your own nic pci
addresses in the echo commands.
*/

ip link set down enp1s0f0
ip link set down enp1s0f0

# unbind from linux
# replace 0000:01:00.0 with your own address.
# use lspci to determine it.
echo 0000:01:00.0 > /sys/bus/pci/drivers/ixgbe/unbind
echo 0000:01:00.1 > /sys/bus/pci/drivers/ixgbe/unbind

# bind to DPDK driver
# replace "8086 10fb" with your own addres
# echo vendor device (lspci -n)
echo "8086 10fb" > /sys/bus/pci/drivers/igb_uio/new_id

/* create router.conf */

nano /etc/router.conf

startup {
  port 0 mtu 1500 tpid 0x8100 state enabled
  port 1 mtu 1500 tpid 0x8100 state enabled

  rx_queue port 0 queue 0 lcore 1
  rx_queue port 0 queue 1 lcore 2
  rx_queue port 0 queue 2 lcore 3

  rx_queue port 1 queue 0 lcore 3
  rx_queue port 1 queue 1 lcore 2
  rx_queue port 1 queue 2 lcore 1

  sysctl set global_packet_counters 1

#  sysctl set arp_cache_timeout 300
}


runtime {
  vif add name p0 port 0 type untagged
  ip addr add 10.0.0.1/24 dev p0

  vif add name p1 port 1 type untagged
  ip addr add 10.0.1.1/24 dev p1

  ip route add 0.0.0.0/0 via 10.0.1.2 src 10.0.1.1

  npf load "/etc/npf.conf.warp17"
}


/* create npf conf */

nano "/etc/npf.conf.warp17"

map p1 netmap 10.0.100.0/24
#alg "icmp"

group default {
  pass stateful final on p1 all
  pass final on p0 all
  #pass final on p1 all
}


/** run the router **/

nano /root/run_router.sh

the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' --syslog='daemon' -n2 -w 0000:01:00.0 -w 0000:01:00.1 -- -c $1
ps -ef | grep the_router | grep -v grep | awk '{print $2}' > /var/run/the_router.pid

run.sh /etc/the_router.conf

