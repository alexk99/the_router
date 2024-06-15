# FRR installation

## Dependencies

	apt -y install \
	   git autoconf automake libtool make libreadline-dev texinfo \
	   pkg-config libpam0g-dev libjson-c-dev bison flex \
	   libc-ares-dev python3-dev python3-jinja2 \
	   install-info build-essential libsnmp-dev perl \
	   libcap-dev python2 libelf-dev libunwind-dev \
	   frr libpcre3-dev cmake libpcre2-dev

## libyang

	cd /usr/src
	git clone https://github.com/CESNET/libyang.git
	cd ./libyang
 	git checkout tags/v2.1.148
	mkdir build; cd build
	cmake -DENABLE_LYD_PRIV=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr \
	  -D CMAKE_BUILD_TYPE:String="Release" ..
	make
	make install
	ldconfig

# FRR 

## Download

Download one of the stable FRR releases from GitHub

	https://github.com/FRRouting/frr/releases/

For example,

	wget https://github.com/FRRouting/frr/archive/refs/tags/frr-9.0.2.tar.gz

## build

	tar xvf ./frr-9.0.2.tar.gz
	cd ./frr-frr-9.0.2/
	./bootstrap.sh
	./configure --localstatedir=/var/run/frr  --runstatedir=/var/run/frr --sysconfdir=/etc/frr --enable-fpm
	make
	make install
	ldconfig

## configure

Configure D_PATH varible in /usr/lib/frr/frrcommon.sh 

	D_PATH="/usr/local/sbin"

Edit /etc/frr/daemons and enable the FRR daemons you need, for example,

	bgpd=yes

Then edit /etc/frr/daemons and 

 * add '--netns=br' option to the 'watchfrr_options' variable
 * add '-M fpm' option to the 'zebra_options' variable

For example

	watchfrr_options="--netns=br"
	zebra_options="  -A 127.0.0.1 -s 90000000 -M fpm"
	
## run

	service frr restart

## check 1

Make sure your KNI interfaces are UP

For example, r_v2002 - is the name of a KNI interface corresponding to the BisonRouter VIF v2002 with KNI flag.

	# ip netns exec br ip link ls dev r_v2002
	8: r_v2002: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
	    link/ether c6:9d:58:8d:6f:9c brd ff:ff:ff:ff:ff:ff


## check 2

Make sure Zebra daemon is up and running using '-M fpm' option

	ps ax | grep zebra | grep fpm
	3230717 ?        S<sl  28:00 /usr/local/sbin/zebra -d -F traditional -A 127.0.0.1 -s 90000000 -M fpm	

## check 3

After BisonRouter has been started FRR should connect to its TCP:2620 port in order to receive routing commands via FPM protocol.

	ip netns exec br netstat -an | grep 2620

