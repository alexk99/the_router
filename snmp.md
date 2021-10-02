## Configure Sudo

Add the content of /etc/bisonrouter/snmpd/sudoers to /etc/sudoers

## Configure NetSNMP

	cp /etc/bisonrouter/snmpd/snmpd.conf.example /etc/snmp/snmpd.conf

Edit community (passwords) in /etc/snmp/snmpd.conf

	rocommunity
	rocommunity6

Restart snmpd service

	service snmpd restart

## SNMP query examples

### PPPoE 

Get pppoe subscriber table

	# snmptable -v2c -c public localhost BISON-ROUTER-MIB::pppoeIpv4Table
	SNMP table: BISON-ROUTER-MIB::pppoeIpv4Table
	
	 pppoeIndex pppoeUsername          pppoeMac pppoeSvid pppoeCvid pppoeSessionId  pppoeIpv4 pppoeMtu pppoeIngressCir pppoeEgressCir pppoeRxPkts pppoeTxPkts pppoeRxOctets pppoeTxOctets pppoePort
	          7         alexk xx:xx:f9:xx:xx:xx       823       400   1633188587-1 10.x.x.x     1492          100000         100000         238         194         26606         53884         2
	          8          alex  xx:xx:b6:xx:xx:xx         0         3   1633188588-2 10.x.x.x     1480          100000         100000          30          30          1743          2943         2

### VIF

	# snmptable -v2c -c public localhost BISON-ROUTER-MIB::vifTable
	SNMP table: BISON-ROUTER-MIB::vifTable
	
	 vifIndex   vifName vifPort vifSvid vifCvid vifRxPkts vifTxPkts vifRxOctets vifTxOctets
	        2        v3       2       0       3        15         6        1301         314
	        3       v20       2       0      20        16        16        1734        1952
	        4 pppoe_any       2     823       0         0         0           0           0
	        5      v100       2       0     100         6         0         550           0

## MIB

Bison Router MIB is located here

        /usr/share/snmp/mibs/BISON-ROUTER-MIB.txt

