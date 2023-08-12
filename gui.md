# GUI installation

## 1
- Install BisonRouter

## 1
- Edit /etc/bisonrouter/brouter.conf and add the following line
to the startup section

        sysctl set kni_driver_type "tap"

- Edit /etc/bisonrouter/brouter.conf and use the 'kni' flag in VIF command
  that creates an interface to access the GUI

- Edit /etc/bisonrouter/bisonrouter.env and add 'br_default_gw' variable.
It should contain an IP address of the BisonRouter default gateway.
For example,

        br_default_gw="192.168.1.2"

- Run GUI configuration

        bisonrouter gui_postinstall

Enter a password for a GUI 'admin' account.
Enter an IP address of a BisonRouter interface to access the GUI.

- Run a bisonrouter nginx server

        service bisonrouter_nginx start

- Run a bisonrouter GUI backend server

        service bisonrouter_gui_backend start

- Open an URL http://IP-address-of-a-BisonRouter
use the 'admin' username and the password that was specified in the 'gui_postinstall' command 
