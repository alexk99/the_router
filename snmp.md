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

  snmptable -v2c -c public localhost BISON-ROUTER-MIB::pppoeIpv4Table

### VIF

  snmptable -v2c -c public localhost BISON-ROUTER-MIB::vifTable
  SNMP table: BISON-ROUTER-MIB::vifTable

   vifIndex   vifName vifPort vifSvid vifCvid vifRxPkts vifTxPkts vifRxOctets vifTxOctets
          2        v3       2       0       3        15         6        1301         314
          3       v20       2       0      20        16        16        1734        1952
          4 pppoe_any       2     823       0         0         0           0           0
          5      v100       2       0     100         6         0         550           0

## MIB

Bison Router MIB is located here

  /usr/share/snmp/mibs/BISON-ROUTER-MIB.txt

