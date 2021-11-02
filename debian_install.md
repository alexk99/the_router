# Install

A 30-day trial build of BisonRouter is available only by request.
Please contact us by email info at bisonrouter.com

Here are installation steps for Ubuntu 20.04.

## Install utilities and libs:

		apt update
		apt -y install python libnuma-dev
		
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

## BisonRouter

### Download BisonRouter 

 Please, contact us at info@bisonrouter.net
 
### Install BisonRouter

		apt install ./bison-router-xxx.deb

### Install DPDK

Use the bisonrouter utility to download and install DPDK.
DPDK will be saved into the directory specified in /etc/bisonrouter/bisonrouter.env
configuration file in the variable br_dpdk_dest. Default DPDK installation path is /usr/src/dpdk-18.11.11.

		bisonrouter install_dpdk

### Configure DPDK ports

Determine what NIC devices are available in the system.
The following command will ouput information about NIC devices and their PCI addresses.

		bisonrouter dev_status


Edit /etc/bisonrouter/bisonrouter.env and save the NIC PCI addresses you want to use 
in the 'br_pci_devs' list.

For example:

		br_pci_devs=(
		  "0000:04:00.0" 
		  "0000:04:00.1"
		)

Run

		bisonrouter bind_devices
		bisonrouter dev_status

Now your PCI devices should be using DPDK drivers and thus should be ready for BisonRouter.

For example:

	Network devices using DPDK-compatible driver
	============================================
	0000:04:00.0 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' drv=vfio-pci unused=ixgbe
	0000:04:00.1 '82599ES 10-Gigabit SFI/SFP+ Network Connection 10fb' drv=vfio-pci unused=ixgbe


### Run BisonRouter

#### Prepare a configuration file

For configuration examples and options see the page <a href="/conf_options2.md#therouter-configuration-file-example">Command reference</a>

Create /etc/bisonrouter/brouter.conf

		nano /etc/bisonrouter/brouter.conf

#### Configure CPU cores

Edit /etc/bisonrouter/bisonrouter.env and update the variable 'br_lcores' according with your hardware setup.
Use only the cores from the same NUMA socket if you system has multiple CPUs. To find out what cores available run:

		bisonrouter cpu_layout


#### Run BisonRouter

Start BisonRouter

		bisonrouter start

Check the syslog to ensure that BisonRouter has started successfully.

 		ROUTER: router configuration file '/etc/bisonrouter/brouter.conf' successfully loaded  

Use the 'rcli' utility to configure and control BisonRouter

		# rcli sh uptime
		Uptime: 0 day(s), 1 hour(s), 38 minute(s), 14 sec(s)
  
To stop/restart BisonRouter also use the bisonrouter utility which supports the following options:
  
		# bisonrouter
		Usage: bisonrouter [OPTIONS]

		Options:

		  start                 - start the Bison Router (BR) daemon
		  stop                  - stop the BR daemon
		  restart               - restart the BR daemon
		  status                - show Bison Router daemon status

		  bind_devices          - load kernel modules and bind PCI devices
		  unbind_devices        - unload kernel modules and unbind devices
		  dev_status            - show device status

		  cpu_layout            - show core and socket information
		  install_dpdk          - fetch the DPDK source code and install
		  reinstall_dpdk        - remove the current DPDK and reinstall
