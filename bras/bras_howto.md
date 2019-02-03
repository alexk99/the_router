Внимание! Перевод не обновляется. Все последние изменения
только в английской версии.

# 1. Введение

В этом howto будет рассмотрена пошаговая настройка bras сервера с помощью программы TheRouter
операционной системы Gentoo Linux и ряда дополнительных программ (Dhcpd, FreeRadius, Mysql, Quagga).

BRAS сервер будет интегрирован в небольшую тестовую сеть и будет выполнять следующие задачи:

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
	  radius_client set secret "secret"
	
	
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

Подробное описание принципа интеграции c Quagga описано
на странице 
<a href="https://github.com/alexk99/the_router/blob/master/quagga_bgp.md#dynamic-routing-integration-with-quagga-routing-suite">
Dynamic routing. Integration with Quagga routing suite
</a>

### 4.4.1. KNI интерфейсы

КNI интерфейсы создаются для каждого vif интерфейса TheRouter,
через который он взаимодействует с другими маршрутиазаторами с помощью какого-либо протокола
динамической маршрутизации. В нашем примере TheRouter устанавливает bgp peer с uplink раутером,
чтобы получить default маршрут.

После запуска TheRouter необходимо поднять KNI интерфейсы и задать им MAC адреса.
МАС адрес KNI интерфейса должен быть равен MAC адресу, соответсвующего ему VIF интерфейса TheRouter.

В нашем примере создается один KNI интефейс c именем rkni_v3 для v3 интерфейса TheRouter.
MAC адрес VIF интерфейса v3 можно увидеть в выводе команды 'rcli sh vif', приведенной выше.

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

Запустив zebra, можно запустить bgpd.

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

Примечание: т.к. в тестовой сети нет дополнительных белых адресов,
поэтому в качества примера используется серая сеть 10.111.0.0/29, и выполняется
дополнительное NAT преобразование на Uplink раутере в единтвенный белый ip адрес, 
настроенный на нем. 

Запуск bgpd

	/etc/init.d/bgpd start
	
Проверим, что default (0.0.0.0/0) маршрут успешно получен и инсталлирован во linux таблицу и в the_router.

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

## 5.1. Настройка TheRouter клиента radius

Команды ниже описывают (в порядке следования) адрес Radius сервера,
ip адрес источника radius запросов, выполняемых TheRouter, 
и общий для клиента и сервера пароль.

	radius_client add server 192.168.20.2
	radius_client add src ip 192.168.20.1
	radius_client set secret "secret"

Ip адрес "src ip" должен быть добавлен на интерфейс TheRouter,
через который доступн radius сервер. В нашем случае это интерфейс v20,
соединяющий TheRouter c linux стэком хоста h4. 

	# link with local linux host
	vif add name v20 port 0 type dot1q cvid 20
	ip addr add 192.168.20.1/24 dev v20

Ip адрес 192.168.20.1 настроен на linux стороне влана 20.

	9: vlan20@eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
	    link/ether 90:e2:ba:4b:b3:17 brd ff:ff:ff:ff:ff:ff
	    inet 192.168.20.2/24 brd 192.168.20.255 scope global vlan20
	       valid_lft forever preferred_lft forever

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
настройка с помощью radius описана в разделе <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber">Vlan per subscriber</a>

### 5.2.2. FreeRadius dictionary

Добавьте в /etc/raddb/dictionary следующие строчки

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

# 6. Настройка и создание сессий/интерфейсов подписчиков (subscriber)

## 6.1. Динамические "Vlan per subscriber" интерфейсы подписчиков

Подробная информация о "vlan per subscriber" интерфейсах описана в 
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#vlan-per-subscriber"
одноименном разделе</a>, а чуть ниже я опишу короткую версию на конкретном примере bras'а, рассматриваемого здесь.

Динамические (создаваемые во время работы TheRouter'а, а не в конфигурационном файле)
интерфейсы создаются на порту. В нашем примере есть всего один порт (port 0), поэтому
чтобы TheRouter знал о небходимости создавать такие интерфейсы у порта указан флаг dynamic_vif.

	port 0 mtu 1500 tpid 0x8100 state enabled flags dynamic_vif bond_slaves 1,2

Встретив на порту пакет, не принадлежащий ни одному из интерфейсов, TheRouter создаст
новый динамический интерфейс, например, для subscriber'а в влане 0.50 (svid.cvid), при условии его успешной
авторизации. 

Авторизация - это отправка запроса и получение ответа c помощью Radius протокола.
Запрос авторизации будет включать все данные о влане subscriber'а, а ответ все данные, необходимые 
для настройки IP интерфейса и маршрутизации (в случае ip unnumbered).

### 6.1.1. Вывод информации о динамических интерфейсах

	h4 ~ # rcli sh subsc
	vlan    subsc ip        mode    port    ingress_car     egress_car      rx_pkts tx_pkts
		...
	0.130                   1       0       200 mbit/s      200 mbit/s      0       0
		...

Список динамических интерфейсов выводится вместе со списком L2/L3 сессий, поэтому некоторые поля
остаются пустыми, например subsc ip, в случае динамических интерфейсов.

### 6.1.2. Проверка маршрутазации для динамических интерфейсов

Т.к. radius ответ авторизации создания динамического интерфейса включает VSA атрибут "therouter_ip_unnumbered",
то TheRouter создает маршрут для IP адреса подписчика, подключенного через этот интерфейс.
Подробнее описание этого процесса находится в разделе <a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#Ответ-на-запрос-авторизации">
Ответ на запрос авторизации</a>

Убедимся, что маршрут создан

	h4 ~ # rcli sh ip route
	...
	10.10.0.11/32 C dev dvif0.130 src 10.10.0.1
	...

Здесь 10.10.0.11 - это ip адрес, указанные в radius ответе. dvif0.130 - это название динамического интерфейса,
где 0.130 - это svid.cvid номер влан.

## 6.2. L2/L3 connected подписчики

### 6.2.1. L2/L3 подписчики
Подробная информация о L2 подписчиках/сессиях описана в разделе
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#l2-connected-subscribers"
L2 connected subscribers</a>, а L3 подписчиков в разделе 
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#l3-connected-subscribers"
L3 connected subscribers</a>

а чуть ниже будет рассмотрены конкретные примеры рассматриваемого здесь BRAS'а.

В отличие от vlan per subscriber случая с динамическими интерфейсами, которые фактически
являются обычными интерфейсам маршрутизатора со своими отдельными настройками IP, для L2/L3
подписчиков не создаются новые интерфейсы, а создаются так назыываемые сессии, которые принадлежат
тому или иному существующему vif интерфейсу TheRouter'а. Поэтому их создание происходит на vif интерфейсе,
у которого указан флаг l2_subs или l3_subs.

В нашем примере L2 сессии создаются на интерфейсе v5, 

	vif add name v5 port 0 type dot1q cvid 5 flags kni,l2_subs

а L3 сессии создаются на интерфейсе v21.

	vif add name v21 port 0 type dot1q cvid 21 flags kni,l3_subs	

Отличие L2 сессий от L3 заключается только в том, что L2 сессия хранит
помимо ip адреса, еще и MAC адрес абонента, инициировавшего сессию. Что
позволит (еще не реализовано) проверять (возможно выборочно) соответсвие MAC адреса
пакетов, проходящих через сессию, MAC адресу, сохраненному в сессии, и в случае нарушения,
выполнять необходимые действия.

Авторизации создания L2/L3 сессий происходит аналогично авторизации создания данамических
интерфейсов, за исключением того, что сессия - не является отдельным интерфейсом, поэтому настройки IP
для нее выполнять не нужно.

В остальном, сессии также поддерживают ограничение трафика, проходящего через нее и PBR маршрутизацию их
трафика.

### 6.2.2. Вывод информации о L2/L3 сессиях

	h4 ~ # rcli sh subsc
	vlan    subsc ip        mode    port    ingress_car     egress_car      rx_pkts tx_pkts
	        192.168.5.132   1       0       200 mbit/s      200 mbit/s      0       0
	        192.168.5.133   1       0       200 mbit/s      200 mbit/s      0       0

# 7. Настройка DHCP и DHCP Relay для выдачи настроек подписчикам

В TheRouter реализована функциональность DHCP Relay.
Единственная доступная на данный момент настройка - это ip адрес DHCP сервера,
которому TheRouter будет перенаправлять все полученные dhcp запросы.

	dhcp_relay 192.168.20.2

В нашем примере указан ip адрес влана 20 linux стэка хоста h4, где запущен
dhcpd сервер.

## 7.1. Конфиг /etc/dhcp/dhcpd.conf
	
Это пример dhcpd конфигурационного файла, в котором описаны настройки
двух subscriber'ов. Dhcpd сервер слушает запросы на интерфейсе v3 (cеть 192.168.20.0)
и идентифицирует двух тестовых subscriber'ов по их mac адресам.

	
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
	
	host c1 {
	  hardware ethernet F8:32:E4:72:61:1B;
	  fixed-address 10.10.0.12;
	}
	
	host c2 {
	  hardware ethernet 00:88:65:36:39:4c;
	  fixed-address 10.10.0.11;
	}
	
## 7.2. Маршрутизация DHCP ответов

Чтобы ответы dhcpd сервера успешно достигли своего назначения должен быть известен маршрут
до сервера, который перенаправляет запросы в dhcpd. В нашем случае dhcpd запросы поступают
с адреса 10.10.0.1 - это IP адрес всех динамических интерфейсов, настроенных по принципу ip
unnumbered и, следовательно, имеющих одинаковый адрес. TheRouter перенаправлят dhcp запросы,
приходящие от подписчиков через динамические интерфейсы, подставляя в них адрес 10.10.0.1,
чтобы dhcpd сервер знал куда обратно следует отправлять ответы.

Это иллюстрируют записи из лог файла dhcpd

	Aug 23 21:07:24 h4 dhcpd[2888]: DHCPREQUEST for 10.10.0.11 (192.168.20.2) from 00:88:65:36:39:4c via 10.10.0.1
	Aug 23 21:07:24 h4 dhcpd[2888]: DHCPACK on 10.10.0.11 to 00:88:65:36:39:4c via 10.10.0.1

в конце строки указано от какого сервера перенапавлен запрос: "via 10.10.0.1".

Поэтому на linux хосте h4 должен быть создан маршрут до адреса 10.10.0.1.
Этот маршрут должен вести в TheRouter, а связь с ним выполнена через влан 20,
поэтому маршрут выглядит вот так:

	ip route add 10.10.0.1 via 192.168.20.1

# 8. Настройка перенаправления заблокированных подписчиков на выделенный сайт

Выполняется с помощью механизма PBR (Policy based routing) и описана в разделе
<a href="https://github.com/alexk99/the_router/blob/master/bras/subsriber_management.md#Управление-трафиком-заблокированных-подписчиков">Управление трафиком заблокированных подписчиков</a>

# 9. Настройка NAT

Настройки NAT описываются в отдельном файле, и загружаются отдельной директивой 

	npf load "/etc/npf.conf.bras_dhcp_relay"

Файл /etc/npf.conf.bras_dhcp_relay

	map v3 netmap 10.111.0.0/29
	
	#alg "icmp"
	
	group default {
	  pass stateful final on v3 all
	}
	
Директива "map v3 netmap 10.111.0.0/29" настраивает NAT преобразование IP адресов источника всех пакетов (входящих,исходящих)
интерфейса v3 в адреса из подсети 10.111.0.0/29. Алгоритм NAT статически связывает исходный адрес источника с новым адресом
из заданной подсети 10.111.0.0/29 следующим образом:
	новый адрес рассчивается как адрес 10.111.0.0 or (32 - 29 == 3 в cтепени 2 == 8) старшие 8 бит адреса исходного адреса.

Т.е. один и тот же внутренний адрес (исходный) будет всегда преобразовываться в один и тот же внешний адрес.
