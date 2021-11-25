## Install the following utilities and libs:

		apt update
		apt -y install g++ libjemalloc-dev libpcap-dev python libpcre2-8-0 autoconf zlib1g-dev flex byacc
		apt -y install cmake libtool libtool-bin subversion rpm libreadline-dev libnuma-dev libnl-genl-3-dev

## Install NPF

		wget https://files.therouter.net/download/npf_build.sh
		./npf_build.sh

## Update system library paths

Add the following lines to the /etc/ld.so.conf.d/router.conf

		/usr/lib64
		/usr/local/lib
		/usr/lib/x86_64-linux-gnu

Run

		ldconfig
