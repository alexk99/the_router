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

PPPoE subscriber table

	# snmptable -v2c -c public localhost BISON-ROUTER-MIB::pppoeIpv4Table
	SNMP table: BISON-ROUTER-MIB::pppoeIpv4Table
	
	pppoeIndex pppoeUsername          pppoeMac pppoeSvid pppoeCvid pppoeSessionId  pppoeIpv4 pppoeMtu pppoeIngressCir pppoeEgressCir pppoeRxPkts pppoeTxPkts pppoeRxOctets pppoeTxOctets pppoePort
	         7         alexk xx:xx:f9:xx:xx:xx       823       400   1633188587-1 10.x.x.x     1492          100000         100000         238         194         26606         53884         2
	         8         alex  xx:xx:b6:xx:xx:xx         0         3   1633188588-2 10.x.x.x     1480          100000         100000          30          30          1743          2943         2

### IPoE 

IPoE subscriber table

	# snmptable -v2c -c public localhost BISON-ROUTER-MIB::ipoeIpv4Table
	
	SNMP table: BISON-ROUTER-MIB::ipoeIpv4Table

	ipoeIndex             ipoeUsername           ipoeMac ipoeSvid ipoeCvid ipoeSessionId     ipoeIpv4 ipoeIngressCir ipoeEgressCir ipoeRxPkts ipoeTxPkts ipoeRxOctets ipoeTxOctets ipoePort ipoeTtl
	        7 1.30.1.1a:xx:xx:xx:xx:xx 1a:b8:xx:xx:xx:xx       30        1  1734623255-7 10.2.168.192         1M;30M        1M;32M          1          1  	 84           84        1     610

### VIF

	# snmptable -v2c -c public localhost BISON-ROUTER-MIB::vifTable
	SNMP table: BISON-ROUTER-MIB::vifTable
	
	 vifIndex   vifName vifPort vifSvid vifCvid vifRxPkts vifTxPkts vifRxOctets vifTxOctets
	        2        v3       2       0       3        15         6        1301         314
	        3       v20       2       0      20        16        16        1734        1952
	        4 pppoe_any       2     823       0         0         0           0           0
	        5      v100       2       0     100         6         0         550           0

### NAT translation state counters

	snmptable -v2c -c xxx localhost BISON-ROUTER-MIB::natTranslationStateCountersTable
	SNMP table: BISON-ROUTER-MIB::natTranslationStateCountersTable

	 natTscMapId natTscMapType natStateUnknown natStateActive natStateSynSent natStateEstablished natStateFinWait natStateCloseWait natStateClosing 	natStateLastAck natStateClosed natStateIcmpActive natStateDns natStateGre
		   1             2               0              0               0                   0               0                 0               0               0              0                  0           0           0
		   1             4               0              0               0                   0               0                 0               0               0              0                  0           0           0

NAT MAP types:

	1 - Deterministic SNAT44
	2 - NAT 1:1
	4 - SNAT44 (PortBlock SNAT44)

### Deterministic NAT statistic

	snmpwalk -v2c -c public localhost BISON-ROUTER-MIB::detSnatStat
	BISON-ROUTER-MIB::detSSTotalMaps.0 = INTEGER: 2
	BISON-ROUTER-MIB::detSSTotalSessions.0 = Counter64: 5
	BISON-ROUTER-MIB::detSSPortmapFailures.0 = Counter64: 0
	BISON-ROUTER-MIB::detSSPortmapFailures2.0 = Counter64: 0
	BISON-ROUTER-MIB::detSSSessionOverflow.0 = Counter64: 0
	BISON-ROUTER-MIB::detSSNoFreePortmapPorts.0 = Counter64: 0
	BISON-ROUTER-MIB::detSSNoFreePortmapPorts.0 = No more variables left in this MIB View (It is past the end of the MIB tree)

### Port utilization

	snmptable -v2c -c xxx localhost BISON-ROUTER-MIB::portRxQueueUtilization
	SNMP table: BISON-ROUTER-MIB::portRxQueueUtilization
	
	 prquIndex prquLcore prquPort prquRxQueue prquEmptyPolls prquPktsPerPoll
	         1         1        0           0            100               0
	         2         1        1           0            100               0
	         3         1        2           0            100               0
	         4         2        0           1            100               0
	         5         2        1           1            100               0
	         6         2        2           1            100               0

### SNAT44 prefix usage

	snmptable -v2c -c xxx localhost BISON-ROUTER-MIB::snat44UsageStat
	SNMP table: BISON-ROUTER-MIB::snat44UsageStat
	
	 snat44UsageStatIndex snat44UsageStatIpStart snat44UsageStatIpEnd snat44UsageStatUsed snat44UsageStatTotal snat44UsageStatPerc snat44UsageStatMapId
	                    1            192.168.2.0        192.168.2.255                   0                  256                   0                    1
	                    2               10.8.0.0           10.8.0.255                   0                  256                   0                    1
	                    3             10.114.0.1           10.114.0.1                   0                 4032                   0                    1
	                    4             10.114.0.3           10.114.0.3                   0                 4032                   0                    1
	                    5             10.114.0.4           10.114.0.4                   0                 4032                   0                    1

### License

	snmpwalk -v2c -c xxx localhost BISON-ROUTER-MIB::licenseInfo
	BISON-ROUTER-MIB::licStatus.0 = INTEGER: 2
	BISON-ROUTER-MIB::licStatusDesc.0 = STRING: OK
	BISON-ROUTER-MIB::licForwBandwidth.0 = INTEGER: 8000000

Status codes

	starting	0
	DMI_OK		1
	OK		2
	failed		3

## MIB

Bison Router MIB is located here

        /usr/share/snmp/mibs/BISON-ROUTER-MIB.txt

