
# Dependencies

	apt -y install frr libjson-c-dev libpcre3-dev libcap-dev bison flex
	apt -y install libelf-dev cmake libpcre2-dev pkg-config texinfo

## libyang

	cd /usr/src
	git clone https://github.com/CESNET/libyang.git
	cd ./libyang
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

	wget https://github.com/FRRouting/frr/archive/refs/tags/frr-8.2.2.tar.gz

## build

	tar xvf ./frr-8.2.2.tar.gz
	cd ./frr-frr-8.2.2/
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

	service frr start
	
## check

Make sure FRR daemons up, running in the 'br' linux namespace, and zebra is listening to FPM port

	ip netns exec br netstat -an | grep 2620

