# GUI installation

### Step 1
- Install BisonRouter

### Step 2
- Edit /etc/bisonrouter/brouter.conf and add the following line
to the startup section

        sysctl set kni_driver_type "tap"

### Step 3
- Edit /etc/bisonrouter/brouter.conf and use the 'kni' flag in VIF command
  that creates an interface to access the GUI. Usually, it is an uplink VIF.

- Edit the 'br_kni_vifs' variable in /etc/bisonrouter/bisonrouter.env and add the names of all VIF with KNI flag into that list.
  For example, if the name of KNI VIF is 'v20' then add 'r_v20' to the 'br_kni_vifs' list.
  Note that you need to use 'r_' prefix when adding VIF names.

### Step 4
- Edit /etc/bisonrouter/bisonrouter.env and add 'br_default_gw' variable.
It should contain an IP address of the BisonRouter default gateway.
For example,

        br_default_gw="192.168.1.2"

### Step 5
- Run GUI configuration

        bisonrouter gui_postinstall

Enter a password for a GUI 'admin' account.
Enter an IP address of a BisonRouter interface to access the GUI.

- Create a GUI 'admin' account with read-write access

        bisonrouter gui_add_user admin xxx read-write

- Optionally create an additional read-only GUI account

        bisonrouter gui_add_user observer xxx read-only

### Step 6
- Run a bisonrouter nginx server

        service bisonrouter_nginx start

### Step 7
- Run a bisonrouter GUI backend server

        service bisonrouter_gui_backend start

### Step 8
- Open an URL http://IP-address-of-a-BisonRouter
use the 'admin' username and the password that was specified in the 'gui_postinstall' command 
