# Source NAT

NAT functionality of the router is based on the NPF standalone lib. So the configuration process here 
is very similar to the configuration of the NetBSD packet filter except that 
a fork of NPF lib is used which has a lot of new code and uses different core for handling
the connection state database.

To describe and measure the perfomance of source nat function of the router a testing lab was build.
The lab includes 3 servers and one L2 switch that connects all the servers into L2 network devided into
2 vlans. From L3 point of view the lab consists of two hosts and a router between them. Host S1 establish
tcp connections to the host S3 and the router perform source NAT for those tcp connections. To established
big number of connections the <a href="https://github.com/Juniper/warp17">Stateful Traffic Generator WARP17</a> 
is used on both hosts S1 and S2.


## Testing lab

<img src="http://therouter.net/images/tests/source_nat/nat_warp_17_lab.png">

## Hardware configuration

### Server - S1
 * Processor: Intel(R) Core(TM) i5-2400 CPU @ 3.10GHz
 * NIC: Intel X520-DA2
 * Ram: 8Gb DDR3
	
### Server - S2
 * Processor: Intel(R) Core(TM) i5-3470 CPU @ 3.20GHz
 * NIC: Intel X520-DA2
 * Ram: 8Gb DDR3
	
### Server - S3
 * Processor: Intel(R) Core(TM) i7-5820K CPU @ 3.30GHz
 * NIC: Intel X520-DA2
 * Ram: 32Gb 4x8

## S2 Router configuration

* /etc/router.conf

	```
	startup {
	  port 0 mtu 1500 tpid 0x8100 state enabled
	  port 1 mtu 1500 tpid 0x8100 state enabled
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	  rx_queue port 0 queue 3 lcore 4
	  rx_queue port 0 queue 4 lcore 5
	
	  rx_queue port 1 queue 0 lcore 5
	  rx_queue port 1 queue 1 lcore 4
	  rx_queue port 1 queue 2 lcore 3
	  rx_queue port 1 queue 3 lcore 2
	  rx_queue port 1 queue 4 lcore 1
	
	  sysctl set global_packet_counters 1
	}
	
	runtime {
	  vif add name p0 port 1 type untagged
	  ip addr add 10.0.0.1/24 dev p0
	
	  vif add name p1 port 0 type untagged
	  ip addr add 10.0.1.1/24 dev p1
	
	  ip route add 0.0.0.0/0 via 10.0.1.2 src 10.0.1.1
	
	  npf load "/etc/npf.conf.warp17"
	}

	```
	
* /etc/npf.conf.warp17
	
	map p1 netmap 10.0.100.0/24
	
	group default {
	  pass stateful final on p1 all
	  pass final on p0 all
	}

## S1 Warp17 - client

	```
	set tests mtu port 0 1500
	add tests l3_intf port 0 ip 10.0.0.2 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.3 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.4 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.5 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.6 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.7 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.8 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.9 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.10 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.11 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.12 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.13 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.14 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.15 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.16 mask 255.255.255.0
	add tests l3_intf port 0 ip 10.0.0.17 mask 255.255.255.0
	add tests l3_gw port 0 gw 10.0.0.1
	
	add tests client tcp port 0 test-case-id 0 src 10.0.0.2 10.0.0.17 sport 1001 60000 dest 10.0.1.2 10.0.1.2 dport 6001 6001
	set tests timeouts port 0 test-case-id 0 init 4
	set tests timeouts port 0 test-case-id 0 uptime infinite
	set tests timeouts port 0 test-case-id 0 downtime 2
	set tests rate port 0 test-case-id 0 open infinite
	set tests rate port 0 test-case-id 0 send infinite
	set tests criteria port 0 test-case-id 0 run-time 600
	set tests client raw port 0 test-case-id 0 data-req-plen 64 data-resp-plen 64
	
	start tests port 0
	show tests ui
	```

## S3 Warp17 - server
	
	```
	add tests l3_intf port 0 ip 10.0.1.2 mask 255.255.255.0
	add tests l3_gw port 0 gw 10.0.1.1
	add tests server tcp port 0 test-case-id 0 src 10.0.1.2 10.0.1.2 sport 6001 6001
	set tests server raw port 0 test-case-id 0 data-req-plen 64 data-resp-plen 64
	start tests port 0
	show tests ui
	```

## Results

### Test in progress...

 * Switch port 26 counters
	
	``` 
	Port Number : 26
	==================================================================
	Frame Size/Type       Frame Counts                  Frames/sec
	---------------       ----------------------        -----------
	64                    87024610                      2815507
	65-127                84094530                      2815504
	128-255               0                             0
	256-511               0                             0
	512-1023              0                             0
	1024-1518             0                             0
	Unicast RX            86007867                      2809947
	Multicast RX          0                             0
	Broadcast RX          0                             0
	
	Frame Type            Total                         Total/sec
	---------------       ----------------------        -----------
	RX Bytes              7970481638                    261333199
	RX Frames   	       86008496                      2809978
	TX Bytes     	       7858637280                    261334242
	TX Frames   	       85110670                      2809971
	```

 * Switch port 7 counters
```
  Port Number : 27
 ==================================================================
 Frame Size/Type       Frame Counts                  Frames/sec
 ---------------       ----------------------        -----------
 64                    350547141                     2813693
 65-127                347636843                     2813692
 128-255               0                             0
 256-511               0                             0
 512-1023              0                             0
 1024-1518             0                             0
 Unicast RX            348611411                     2809496
 Multicast RX          0                             0
 Broadcast RX          19                            0

 Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              32365232450                   261001311
 RX Frames   	       348612008                     2809484
 TX Bytes     	       32481488302                   261000601
 TX Frames   	       349571993                     2809480
```

 * Warp client 
 ```
 +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
|Port 0                                                                             |Port 0: link UP, speed 10Gbps, duplex full(manual), TX: 19.09% RX: 19.09%
|TcId        Type             Criteria      State    Runtime          Quick stats   |============================================================================================================================
|================================================================================== |  CL s/s  Established       Closed         Data
|   0      TCP CL  Run Time:      600s    RUNNING    157.20s    Run Time:    157s   | TCP/UDP            0            0      1407040
|                                                                                   |
|                                                                                   |============================================================================================================================
|                                                                                   |TCP SM Stats
|                                                                                   |============================================================================================================================
|                                                                                   |INIT       :                0      LISTEN     :                0
|                                                                                   |SYN_SENT   :                0      SYN_RECV   :                0
|                                                                                   |ESTAB      :           944000      FIN_WAIT_1 :                0
|                                                                                   |FIN_WAIT_2 :                0      LAST_ACK   :                0
|                                                                                   |CLOSING    :                0      TIME_WAIT  :                0
|                                                                                   |CLOSE_WAIT :                0      CLOSED     :                0
|                                                                                   |
|                                                                                   |============================================================================================================================
|                                                                                   |IP Stats
|                                                                                   |============================================================================================================================
|                                                                                   |Rx Pkts     :        431074214  Invalid Cksum   :      0
|                                                                                   |Rx Bytes    :      30975894896  Small Mbuf      :      0
|                                                                                   |Rx ICMP     :                0  Small Hdr       :      0
|                                                                                   |Rx TCP      :        431074214  Invalid Len     :      0
|-----------------------------------------------------------------------------------|Rx UDP      :                0  Rx Frags        :      0
|Port 0, Test Case 0                                                                |Rx Other    :                0
|                                                                                   |Invalid Ver :                0  Res Bit         :      0
|Statistics:                                                                        |
|      Estab/s      Closed/s   Data Send/s                                          |============================================================================================================================
|            0             0       1407040                                          |Link Stats
|     Requests       Replies                                                        |============================================================================================================================
|    214939234     213995246                                                        |                      Link       Rate              SW
|                                                                                   |   Rx Pkts       431073821    2810955       431074234
|                                                                                   |  Rx Bytes     38309881648  250175093     38309918532
|                                                                                   |   Tx Pkts       432033873    2810950       432034182
|                                                                                   |  Tx Bytes     38422304376  250174444     37123258996
|                                                                                   |    Rx Err               0          0             N/A
|                                                                                   |    Tx Err               0          0               0
|                                                                                   |Rx No Mbuf               0          0             N/A
|                                                                                   |
 ```

 * Warp server
```
 +-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
|Port 0                                                                             |Port 0: link UP, speed 10Gbps, duplex full(manual), TX: 19.08 |Port 1: link DOWN, speed 0Mbps, duplex N/A(manual), TX: 0.00%
|TcId        Type             Criteria      State    Runtime          Quick stats   |============================================================= |=============================================================
|================================================================================== |  CL s/s  Established       Closed         Data               |  CL s/s  Established       Closed         Data
|   0     TCP SRV    Srv Up:         1     PASSED    180.94s      Srv Up:       1   | TCP/UDP            0            0      1403773               | TCP/UDP            0            0            0
|Port 1                                                                             |                                                              |
|TcId        Type             Criteria      State    Runtime          Quick stats   |============================================================= |=============================================================
|================================================================================== |TCP SM Stats                                                  |TCP SM Stats
|                                                                                   |============================================================= |=============================================================
|                                                                                   |INIT       :                0      LISTEN     :               |INIT       :                0      LISTEN     :
|                                                                                   |SYN_SENT   :                0      SYN_RECV   :               |SYN_SENT   :                0      SYN_RECV   :
|                                                                                   |ESTAB      :           944000      FIN_WAIT_1 :               |ESTAB      :                0      FIN_WAIT_1 :
|                                                                                   |FIN_WAIT_2 :                0      LAST_ACK   :               |FIN_WAIT_2 :                0      LAST_ACK   :
|                                                                                   |CLOSING    :                0      TIME_WAIT  :               |CLOSING    :                0      TIME_WAIT  :
|                                                                                   |CLOSE_WAIT :                0      CLOSED     :               |CLOSE_WAIT :                0      CLOSED     :
|                                                                                   |  0                                                           |  0
|                                                                                   |============================================================= |=============================================================
|                                                                                   |IP Stats                                                      |IP Stats
|                                                                                   |============================================================= |=============================================================
|                                                                                   |Rx Pkts     :        507430261  Invalid Cksum   :      0      |Rx Pkts     :                0  Invalid Cksum   :      0
|                                                                                   |Rx Bytes    :      36503736456  Small Mbuf      :      0      |Rx Bytes    :                0  Small Mbuf      :      0
|                                                                                   |Rx ICMP     :                0  Small Hdr       :      0      |Rx ICMP     :                0  Small Hdr       :      0
|                                                                                   |Rx TCP      :        507430261  Invalid Len     :      0      |Rx TCP      :                0  Invalid Len     :      0
|-----------------------------------------------------------------------------------|Rx UDP      :                0  Rx Frags        :      0      |Rx UDP      :                0  Rx Frags        :      0
|Port 0, Test Case 0                                                                |Rx Other    :                0                                |Rx Other    :                0
|                                                                                   |Invalid Ver :                0  Res Bit         :      0      |Invalid Ver :                0  Res Bit         :      0
|Statistics:                                                                        |                                                              |
|      Estab/s      Closed/s   Data Send/s                                          |============================================================= |=============================================================
|            0             0       1403773                                          |Link Stats                                                    |Link Stats
|     Requests       Replies                                                        |============================================================= |=============================================================
|    253088595     252144654                                                        |                      Link       Rate              SW         |                      Link       Rate              SW
|                                                                                   |   Rx Pkts       507429788    2810313       507430262         |   Rx Pkts               0          0               0
|                                                                                   |  Rx Bytes     45132938602  250117941     45132979922         |  Rx Bytes               0          0               0
|                                                                                   |   Tx Pkts       506532071    2810313       506532396         |   Tx Pkts               0          0               0
|                                                                                   |  Tx Bytes     45024694482  250117942     43499265892         |  Tx Bytes               0          0               0
|                                                                                   |    Rx Err               0          0             N/A         |    Rx Err               0          0             N/A
|                                                                                   |    Tx Err               0          0               0         |    Tx Err               0          0               0
|                                                                                   |Rx No Mbuf               0          0             N/A         |Rx No Mbuf               0          0             N/A
|                                                                                   |                                                              |
```

### Test is completed

 * Port 26
``` 
DGS-1510-28X/ME:admin#show packet ports 26
Command: show packet ports 26


 Port Number : 26
 ==================================================================
 Frame Size/Type       Frame Counts                  Frames/sec
 ---------------       ----------------------        -----------
 64                    1677113329                    0
 65-127                1675070297                    0
 128-255               0                             0
 256-511               0                             0
 512-1023              0                             0
 1024-1518             0                             0
 Unicast RX            1676097213                    0
 Multicast RX          0                             0
 Broadcast RX          5                             0

 Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              155810310186                  0
 RX Frames   	       1676097218                    0
 TX Bytes     	       155883519104                  0
 TX Frames   	       1676086408                    0
```

 * Port 27
 ```
 DGS-1510-28X/ME:admin#show packet ports 27
Command: show packet ports 27


 Port Number : 27
 ==================================================================
 Frame Size/Type       Frame Counts                  Frames/sec
 ---------------       ----------------------        -----------
 64                    1677132153                    0
 65-127                1674906127                    0
 128-255               0                             0
 256-511               0                             0
 512-1023              0                             0
 1024-1518             0                             0
 Unicast RX            1675841274                    0
 Multicast RX          0                             0
 Broadcast RX          138                           0

 Frame Type            Total                         Total/sec
 ---------------       ----------------------        -----------
 RX Bytes              155856494618                  0
 RX Frames   	       1675841412                    0
 TX Bytes     	       155818510668                  0
 TX Frames   	       1676196868                    0 
 ```

 * Warp client
 ```
+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
|Port 0                                                                             |Port 0: link UP, speed 10Gbps, duplex full(manual), TX: 0.00% RX: 0.00%6%
|TcId        Type             Criteria      State    Runtime          Quick stats   |============================================================================================================================
|================================================================================== |  CL s/s  Established       Closed         Data
|   0      TCP CL  Run Time:      600s     PASSED    600.36s    Run Time:    600s   | TCP/UDP            0            0            0
|                                                                                   |
|                                                                                   |============================================================================================================================
|                                                                                   |TCP SM Stats
|                                                                                   |============================================================================================================================
|                                                                                   |INIT       :                0      LISTEN     :                0
|                                                                                   |SYN_SENT   :                0      SYN_RECV   :                0
|                                                                                   |ESTAB      :                0      FIN_WAIT_1 :                0
|                                                                                   |FIN_WAIT_2 :                0      LAST_ACK   :                0
|                                                                                   |CLOSING    :                0      TIME_WAIT  :                0
|                                                                                   |CLOSE_WAIT :                0      CLOSED     :                0
|                                                                                   |
|                                                                                   |============================================================================================================================
|                                                                                   |IP Stats
|                                                                                   |============================================================================================================================
|                                                                                   |Rx Pkts     :       1675196132  Invalid Cksum   :      0
|                                                                                   |Rx Bytes    :     120597080096  Small Mbuf      :      0
|                                                                                   |Rx ICMP     :                0  Small Hdr       :      0
|                                                                                   |Rx TCP      :       1675196132  Invalid Len     :      0
|-----------------------------------------------------------------------------------|Rx UDP      :                0  Rx Frags        :      0
|Port 0, Test Case 0                                                                |Rx Other    :                0
|                                                                                   |Invalid Ver :                0  Res Bit         :      0
|Statistics:                                                                        |
|      Estab/s      Closed/s   Data Send/s                                          |============================================================================================================================
|            0             0             0                                          |Link Stats
|     Requests       Replies                                                        |============================================================================================================================
|    836880606     835936606                                                        |                      Link       Rate              SW
|                                                                                   |   Rx Pkts      1675196270          0      1675196271
|                                                                                   |  Rx Bytes    149077020252          0    149077020312
|                                                                                   |   Tx Pkts      1676196843          0      1676196868
|                                                                                   |  Tx Bytes    149113721696          0    144078117044
|                                                                                   |    Rx Err               0          0             N/A
|                                                                                   |    Tx Err               0          0               0
|                                                                                   |Rx No Mbuf               0          0             N/A 
 ```
 
 * Warp server
 ```
+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
|Port 0                                                                                       |Port 0: link UP, speed 10Gbps, duplex full(manual), TX: 0.00% RX: 0. |Port 1: link DOWN, speed 0Mbps, duplex N/A(manual), TX: 0.00% RX: 0.
|TcId        Type             Criteria      State    Runtime          Quick stats             |==================================================================== |====================================================================
|============================================================================================ |  CL s/s  Established       Closed         Data                      |  CL s/s  Established       Closed         Data
|   0     TCP SRV    Srv Up:         1     PASSED    639.80s      Srv Up:       1             | TCP/UDP            0            0            0                      | TCP/UDP            0            0            0
|Port 1                                                                                       |                                                                     |
|TcId        Type             Criteria      State    Runtime          Quick stats             |==================================================================== |====================================================================
|============================================================================================ |TCP SM Stats                                                         |TCP SM Stats
|                                                                                             |==================================================================== |====================================================================
|                                                                                             |INIT       :                0      LISTEN     :                2     |INIT       :                0      LISTEN     :                0
|                                                                                             |SYN_SENT   :                0      SYN_RECV   :                0     |SYN_SENT   :                0      SYN_RECV   :                0
|                                                                                             |ESTAB      :                0      FIN_WAIT_1 :                0     |ESTAB      :                0      FIN_WAIT_1 :                0
|                                                                                             |FIN_WAIT_2 :                0      LAST_ACK   :                0     |FIN_WAIT_2 :                0      LAST_ACK   :                0
|                                                                                             |CLOSING    :                0      TIME_WAIT  :                0     |CLOSING    :                0      TIME_WAIT  :                0
|                                                                                             |CLOSE_WAIT :                0      CLOSED     :                0     |CLOSE_WAIT :                0      CLOSED     :                0
|                                                                                             |                                                                     |
|                                                                                             |==================================================================== |====================================================================
|                                                                                             |IP Stats                                                             |IP Stats
|                                                                                             |==================================================================== |====================================================================
|                                                                                             |Rx Pkts     :       1675674304  Invalid Cksum   :      0             |Rx Pkts     :                0  Invalid Cksum   :      0
|                                                                                             |Rx Bytes    :     120588448832  Small Mbuf      :      0             |Rx Bytes    :                0  Small Mbuf      :      0
|                                                                                             |Rx ICMP     :                0  Small Hdr       :      0             |Rx ICMP     :                0  Small Hdr       :      0
|                                                                                             |Rx TCP      :       1675674304  Invalid Len     :      0             |Rx TCP      :                0  Invalid Len     :      0
|                                                                                             |Rx UDP      :                0  Rx Frags        :      0             |Rx UDP      :                0  Rx Frags        :      0
|---------------------------------------------------------------------------------------------|Rx Other    :                0                                       |Rx Other    :                0
|Port 0, Test Case 0                                                                          |Invalid Ver :                0  Res Bit         :      0             |Invalid Ver :                0  Res Bit         :      0
|                                                                                             |                                                                     |
|Statistics:                                                                                  |==================================================================== |====================================================================
|      Estab/s      Closed/s   Data Send/s                                                    |Link Stats                                                           |Link Stats
|            0             0             0                                                    |==================================================================== |====================================================================
|     Requests       Replies                                                                  |                      Link       Rate              SW                |                      Link       Rate              SW
|    836881590     836881259                                                                  |   Rx Pkts      1675674309          0      1675674310                |   Rx Pkts               0          0               0
|                                                                                             |  Rx Bytes    149080546774          0    149080546834                |  Rx Bytes               0          0               0
|                                                                                             |   Tx Pkts      1676086405          0      1676086408                |   Tx Pkts               0          0               0
|                                                                                             |  Tx Bytes    149179173292          0    144151688272                |  Tx Bytes               0          0               0
|                                                                                             |    Rx Err               0          0             N/A                |    Rx Err               0          0             N/A
|                                                                                             |    Tx Err               0          0               0                |    Tx Err               0          0               0
|                                                                                             |Rx No Mbuf               0          0             N/A                |Rx No Mbuf               0          0             N/A
|                                                                                             |                                                                     |
 ```

# Conclusion
The router is able to perform source NAT function for traffic rate of 5.4 Mpps for 0.9M simultaneous tcp connections.