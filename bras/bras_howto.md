# 1. Введение

В этом howto будет рассмотрена пошаговая настройка bras сервера с помощью программы TheRouter
операционной системы Gentoo Linux и ряда дополнительных программ (Dhcpd, FreeRadius, Mysql, Quagga).

BRAS сервер будет интегрирован в небольшую тестовую сеть будет выполнять следующие задачи:

 * терминация пользовательских ethernet qinq вланов;
 * авторация и аутентификация пользователей с помощью протокола Radius;
 * перенаправление трафика заблокированных пользователей на cпециальный веб сайт;
 * ограничение скорости пользователей;
 * маршрутизацию пользовательского трафика с помощью динамических протоколов маршрутизации;
 * пребразование серых адресов пользователей в pool белых адресов с помощью NAT;
	
# 2. Настройка операционной системы Gentoo и установка TheRouter

Следуя <a href="https://github.com/alexk99/the_router/blob/master/install.md">инструкции</a>
установите DPDK и TheRouter на машину с операционной системой Gentoo Linux.

# 3. Схема сети, в которой будет установлен BRAS.

Сеть состоит из linux хоста H4, на котором установлен TheRouter и необходимые для
его работы программы (Dhcpd, FreeRadius, Mysql, Quagga), пограничного uplink маршрутизатора,
подключенного к интернет, и подписчиков Subscriber 1-4. Подписчики 1 и 2 подключены через
отдельные вланы, 3-4 через L2 broadcast domain.

## 3.1. L2 схема сети

<img src="http://therouter.net/images/bras/bras howto l2.png">

## 3.2. L3 схема сети

<img src="http://therouter.net/images/bras/bras_howto_l3.png">

Два основных статических интерфейса the_router, настроенных в конфигурационном файле, это
интерфейс v3 и v20. v3 связывает the_router с его единственным uplink'ом. v20 связывыает
сетевой стэк the_router (radius client TheRouter'а и т.п.) с сетевым стэком linux, который
работает на том же хосте h4, но использует свои сетевые карты. 

# 4. Запуск TheRouter, проверка L2 связности и маршрутизации.

## 4.1. Запуск TheRouter

the_router --proc-type=primary -c 0xF --lcores='0@0,1@1,2@2,3@3' \
 --syslog='daemon' -n2 -w 0000:60:00.0 -w 0000:60:00.1  --vdev 'eth_bond0,mode=4,slave=0000:60:00.0,slave=0000:60:00.1,xmit_policy=l23' \
 -- -c /etc/router_bras_dhcp_relay_lag.conf

## 4.2. Конфигурационный файл TheRouter

Ниже приведен текст конфигурационного файла TheRouter.
Подробный разбор директив из этого файла приведен в последующих разделах этого howto.

/etc/router_bras_dhcp_relay_lag.conf

	startup {
	  sysctl set mbuf 8192
	  sysctl set log_level 7
	
	  # LAG (slave ports 0,1)
	  port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 1,2
	
	  rx_queue port 0 queue 0 lcore 1
	  rx_queue port 0 queue 1 lcore 2
	  rx_queue port 0 queue 2 lcore 3
	
	  sysctl set global_packet_counters 1
	  sysctl set arp_cache_timeout 300
	  sysctl set arp_cache_size 65536
	  sysctl set dynamic_vif_ttl 600
	
	  sysctl set dhcp_relay_enabled 1
	}
	
	runtime {
	  # blackhole multicast addresses
	  ip route add 224.0.0.0/4 unreachable
	
	  radius_client add src ip 192.168.20.1
	
	
	  ip route add 10.10.0.0/24 unreachable
	
	  # link with local linux host
	  vif add name v20 port 0 type dot1q cvid 20
	  ip addr add 192.168.20.1/24 dev v20
	
	  # home network link (vlan3)
	  vif add name v3 port 0 type dot1q cvid 3 flags npf_on, kni
	  ip addr add 192.168.1.112/24 dev v3
	  #ip route add 0.0.0.0/0 via 192.168.1.3 src 192.168.1.112
	
	  # cisco L3 connected
	  vif add name v21 port 0 type dot1q cvid 21 flags kni,l3_subs
	  ip addr add 192.168.21.1/24 dev v21
	
	  # L2 connected (zyxel 5Ghz WiFi)
	  vif add name v5 port 0 type dot1q cvid 5 flags kni,l2_subs
	  ip addr add 192.168.5.1/24 dev v5
	
	
	  ## static arp records
	  # radius server
	  arp add 192.168.20.2 90:e2:ba:4b:b3:17 dev v20 static
	
	  dhcp_relay 192.168.20.2
	
	  radius_client add server 192.168.20.2
	  radius_client set secret "hokawuwe"
	
	
	  # PBR
	  ip route table add rt_bl
	
	  u32set create ips1 size 4096 bucket_size 16
	  u32set create l2s1 size 4096 bucket_size 16
	  subsc u32set init ips1 l2s1
	
	  ip pbr rule add prio 10 u32set ips1 type "ip" table rt_bl
	  ip pbr rule add prio 20 u32set l2s1 type "l2" table rt_bl
	
	
	  # NPF
	  # npf load "/etc/npf.conf.accept_all_2"
	
	  npf load "/etc/npf.conf.bras_dhcp_relay"
	}

## 4.3. Проверка связности

### 4.3.1. Проверим, что созданы все описанные в конфигурационном файле интерфейсы

	h4 src # rcli sh vif
	name    port    vid     mac                     type    flags   idx     ingress_car     egress_car
	v20     0       0.20    00:1B:21:3C:69:44       dot1q           10      -       -
	v5      0       0.5     00:1B:21:3C:69:44       dot1q   kni,l2 subs     13      -       -
	v3      0       0.3     00:1B:21:3C:69:44       dot1q   kni,NPF 11      -       -

### 4.3.2. ARP

	h4 src # rcli sh arp
	port    vid     ip      mac     type    state
	0       0.20    192.168.20.2    90:E2:BA:4B:B3:17       static  ok
	0       0.5     192.168.5.124   A8:5B:78:09:0C:E1       dynamic ok
	0       0.3     192.168.1.3     D4:CA:6D:7C:D0:DC       dynamic ok

### 4.3.3. ICMP

	h4 src # rcli ping -c3 192.168.1.3
	Ping 192.168.1.3 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.283 ms
	reply 56 bytes icmp_seq=2 time=0.279 ms
	reply 56 bytes icmp_seq=3 time=0.278 ms
	---
	Ping 192.168.1.3 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.278/0.280/0.283

	h4 src # rcli ping -c3 192.168.20.2
	Ping 192.168.20.2 56(84) bytes of data.
	reply 56 bytes icmp_seq=1 time=0.501 ms
	reply 56 bytes icmp_seq=2 time=0.557 ms
	reply 56 bytes icmp_seq=3 time=0.279 ms
	---
	Ping 192.168.20.2 statistics:
	sent: 3, recv: 3 (100%), lost: 0 (0%)
	round-trip min/avg/max = 0.279/0.445/0.557

## 4.4. Запуск KNI интерфейсов и интеграция с Quagga

### 4.4.1. Поднимем KNI интерфейсы. КNI интерфейсы создаются для каждого vif интерфейса TheRouter,
через который он взаимодействует с другими маршрутиазаторами с помощью какого-либо протокола
динамической маршрутизации. В нашем примере TheRouter устанавливает bgp peer с uplink раутером,
чтобы получить default маршрут.

	#!/bin/bash
	ip link set up rkni_v3
	ip link set dev rkni_v3 address 00:1B:21:3C:69:44

### 4.4.2. Запуск quagga

Сначала необходимо настроить и запустить zebra.

Конфигурационный файл /etc/quagga/zebra.conf

	hostname h4
	
	! Set both of these passwords
	password xxx
	enable password xxx
	
	# rt1
	table 250
	
	! Turn off welcome messages
	no banner motd
	log file /var/log/quagga/zebra.log

Единственный важдный момент в этом файле - это директива table 250, которая говорит
zebrа инсталлировать все полученные от демонов маршрутизации маршруты не в основную
таблицу маршрутизации linux, а в таблицу c id 250. Id таблица должна быть заранее
прописан в файле /etc/iproute2/rt_tables

	250     rt1

Это необходимо, чтобы полученные маршруты не мешали работе linux, т.к. они предназначены
исключительно для TheRouter и будут переданы ему через "zebra FIB push interface" интерфейс.

Запуск zebra
	/etc/init.d/zebra start

Запустив zebra можно запустить bgpd.

Конфигурационный файл /etc/quagga/bgpd.conf

	h4# cat /etc/quagga/bgpd.conf
	!
	! Zebra configuration saved from vty
	!   2017/05/09 15:50:41
	!
	hostname h4
	password xxx
	log syslog
	!
	router bgp 64512
	 bgp router-id 192.168.1.112
	 network 10.111.0.0/29
	 neighbor 192.168.1.3 remote-as 64513
	 exit
	!
	line vty
	!

Здесь описан единственный пир c uplink маршрутизатором 192.168.1.3
и анонс сети 10.111.0.0/29, которая будет использована для NAT преобразования
ip адресов подписчиков в внешние адреса маршрутизируемые в интернет.

Примечание: т.к. в тестовой сети не было дополнительных белых адресов,
поэтому в качества примера используется серая сеть 10.111.0.0/29, и выполняется
дополнительное NAT преобразование на Uplink раутере в единтвенный белый ip адрес, 
настроенный на нем. 

Запуск bgpd
	/etc/init.d/bgpd start
	
Проверим, что маршрут успешно получен и инсталлирован во linux таблицу и в the_router.

Linux таблица rt1

	h4 src # ip route ls table rt1
	default via 192.168.1.3 dev rkni_v3  proto zebra  metric 20

Основная таблица маршрутизации TheRouter:

	h4 src # rcli sh ip route
	224.0.0.0/4 is unreachable
	192.168.21.0/24 C dev v21 src 192.168.21.1
	192.168.5.0/24 C dev v5 src 192.168.5.1
	192.168.20.0/24 C dev v20 src 192.168.20.1
	192.168.1.0/24 C dev v3 src 192.168.1.112
	10.10.0.0/24 is unreachable
	0.0.0.0/0 via 192.168.1.3 dev v3 src 192.168.1.112
	

# 5. Настройка Radius.

## 5.1. Настройка TheRouter клиента radius.

Команды ниже описывают (в порядке следования) адрес Radius сервера,
ip адрес источника radius запросов, выполняемых TheRouter, 
и общий для клиента и сервера пароль.

	radius_client add server 192.168.3.2
	radius_client add src ip 192.168.3.1
	radius_client set secret "secret"


Ip адрес "src ip" должен быть на интерфейс TheRouter,
через который доступн radius сервер. В нашем случае это интерфейс v20,
соединяющий TheRouter c linux стэком хоста h4. Ip адрес 192.168.3.2 настроен
на linux стороне влана 20.

  # link with local linux host
  vif add name v20 port 0 type dot1q cvid 20
  ip addr add 192.168.20.1/24 dev v20

## 5.2. Настройка Radius сервера на стороне Linux

В качестве radius сервера используется пакет FreeRadius.
Я не буду полностью описывать его настройку, т.к. у FreeRadius проекта есть своя документация и 
в интернет есть множество примеров его использования.

Приведу лишь текст основного sql запроса, иллюстрирующий использование radius атрибутов TheRouter,
и cписок VSA TheRouter'а (dictionary).

/etc/raddb/sql/mysql/dialup.conf

	## router_bras_dhcp_relay.conf
	## pbr.
	authorize_reply_query = "SELECT 1, '%{SQL-User-Name}', 'therouter_ingress_cir', '200', '=' \
	UNION SELECT 2, '%{SQL-User-Name}', 'therouter_engress_cir', '200', '+=' \
	UNION SELECT 3, '%{SQL-User-Name}', 'therouter_ipv4_addr', GetIpoeUserService(%{request:therouter_port_id}, '%{request:therouter_outer_vid}', '%{request:therouter_inner_vid}'), '+=' \
	UNION SELECT 4, '%{SQL-User-Name}', 'therouter_ipv4_mask', '24', '+=' \
	UNION SELECT 5, '%{SQL-User-Name}', 'therouter_ip_unnumbered', '1', '+='"

Этот sql запрос будет использован FreeRadius'ом для формирования ответа на 
radius запрос TheRouter'а об авторизации подписчика, подключенного через отдельный влан.
Mysql хранимая процедура GetIpoeUserService рассчитает ip адрес подписчика на основе порта и id влана,
через который подписчик подключен к TheRouter. Информация radius ответа будет использована TheRouter'ом для
настройки маршрутизации трафика в сторону подписчика по принципу ip unnumbered.

Подробно механизм ip unnumbered для динамических "Vlan per subscriber" интерфейсов и их
настройка с помощью radius описана по следующей ссылке в разделе Vlan per subscriber

https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber

### 5.2.2. FreeRadius dictionary

	VENDOR       TheRouter     12345
	BEGIN-VENDOR TheRouter
	    ATTRIBUTE therouter_ingress_cir 1 integer
	    ATTRIBUTE therouter_engress_cir 2 integer
	    ATTRIBUTE therouter_ipv4_addr 3 integer
	    ATTRIBUTE therouter_ipv4_mask 4 integer
	    ATTRIBUTE therouter_outer_vid 5 integer
	    ATTRIBUTE therouter_inner_vid 6 integer
	    ATTRIBUTE therouter_ip_unnumbered 7 integer
	    ATTRIBUTE therouter_port_id 8 integer
	    ATTRIBUTE therouter_ipv4_gw 9 integer
	    ATTRIBUTE therouter_pbr 10 integer
	END-VENDOR   TheRouter

# 6. Настройка динамических qinq, dot1q интерфейсов подписчиков

Подробная информация о "Vlan per subscriber" интерфейсах описана в разделе
https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber

# 7. Настройка DHCP и DHCP Relay для выдачи настроек подписчикам

В TheRouter реализована функциональность DHCP Relay.
Единственная доступная на данный момент настройка - это ip адрес DHCP сервер,
которому TheRouter будет перенаправлять все полученные dhcp запросы.

	dhcp_relay 192.168.20.2

В нашем примере указан ip адрес влана 20 linux стэка хоста h4, где запущен
dhcpd сервер.

	конфиг dhcpd.conf
	
	#
	# Global parameters
	#
	default-lease-time 864000; # 10 days
	max-lease-time 864000;
	option domain-name "home";
	option domain-name-servers 192.168.1.3;
	ddns-update-style none;
	ddns-updates off;
	authoritative;
	
	shared-network dd {
	
	subnet 10.10.0.0 netmask 255.255.255.0 {
	    option subnet-mask 255.255.255.0;
	    option routers 10.10.0.1;
	
	    pool {
	        range 10.10.0.50 10.10.0.200;
	        default-lease-time 72000; # 20 hours
	        max-lease-time 72000;
	        allow unknown clients;
	    }
	}
	
	subnet 192.168.20.0 netmask 255.255.255.0 {
	}
	
	}
	
	# 0 - hostname, 1 - host ip, 2 - host mac
	
	## alexk computer
	host c1 {
	  hardware ethernet F8:32:E4:72:61:1B;
	  fixed-address 10.10.0.12;
	}
	
	host c2 {
	  hardware ethernet 00:88:65:36:39:4c;
	  fixed-address 10.10.0.11;
	}

!!?!? нужен ли маршрут

# 8. Настройка перенаправления заблокированных подписчиков на выделенный сайт

Выполняется с помощью механизма PBR (Policy based routing) и описана в разделе
"Управление трафиком заблокированных подписчиков"
https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#Управление-трафиком-заблокированных-подписчиков

# 9. Настройка NAT

# 10. Заключение

todo