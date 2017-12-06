# Использование TheRouter в качестве одного из core маршрутизаторов в сети оператора связи.

Месяц назад 15 мая 2017 было принято решение запустить первую боевую инсталляцию 
TheRouter в качестве одного из core маршрутизаторов на сети оператора связи, который
насчитывает более 60 тысяч подключений домашних пользователей

Новый маршрутизатор
под управлением TheRouter должен был решить следующие задачи:

- марштутизация трафика пользователей сети от bras'ов к Google cache
- маршрутизация трафика Google cache в интернет через пограничный маршрутизатор
- динамическая маршрутизация с помощью bgp
- получение bgp full view от пограничного маршрутазатора

Эти задачи и место расположения TheRouter в сети были были выбраны с учетом того,
что проект TheRouter находится в режиме эксперимента и до этого момента ни разу не
использовался для решения боевых задач на реальных сетях. Предложенная
же техническим директором оператора связи схема применения TheRouter совершенно безболезненна для 
для его сети и предполагает любые остановки the_router'а, необходимые для отладки и обновления.

# Характеристики машины TheRouter

Сервер HP ProLiant DL380 G6, 2 процессора Intel 6C X5650 2.66 GHz, 48GB DRAM
Cетевые карты: Intel XL710
Note: используется только один из процессоров машины чтобы избежать "медленного" NUMA взаимодействия.

# Настройка TheRouter

* /etc/router.conf

		startup {
		  # mbuf mempool size
		  sysctl set mbuf 16384
		
		  port 0 mtu 1500 tpid 0x8100 state enabled
		  port 1 mtu 1500 tpid 0x8100 state enabled
		  port 2 mtu 1500 tpid 0x8100 state enabled
		  port 3 mtu 1500 tpid 0x8100 state enabled
		
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
		
		  rx_queue port 2 queue 0 lcore 1
		  rx_queue port 2 queue 1 lcore 2
		  rx_queue port 2 queue 2 lcore 3
		  rx_queue port 2 queue 3 lcore 4
		  rx_queue port 2 queue 4 lcore 5
		
		  rx_queue port 3 queue 0 lcore 5
		  rx_queue port 3 queue 1 lcore 4
		  rx_queue port 3 queue 2 lcore 3
		  rx_queue port 3 queue 3 lcore 2
		  rx_queue port 3 queue 4 lcore 1
		
		  sysctl set global_packet_counters 1
		}
		
		
		runtime {
		  # blackhole multicast addresses
		  ip route add 224.0.0.0/4 unreachable
		
		  # nas clients
		  ip route add xxx.xxx.0/21 unreachable
		  ip route add xxx.xxx.0/22 unreachable
		  ip route add xxx.xxx.0/21 unreachable
		  ip route add xxx.xxx.0/22 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/20 unreachable
		  ip route add xxx.xxx.0/21 unreachable
		
		  vif add name p0 port 0 type untagged flags kni
		  ip addr add xxx.xxx.92/28 dev p0
		
		  vif add name p2 port 2 type untagged flags kni
		  ip addr add xxx.xxx.97/27 dev p2
		
		  # default to mx
		  ip route add 0.0.0.0/0 via xxx.xxx.81 src xxx.xxx.92
		}


### Схема сети
<img src="http://therouter.net/images/production/bzn/bizin.png">

## Результаты и статистика

### Кол-во изученных маршуртов

		the_router ~ # rcli sh ip route | wc -l
		662055

### Last 7 days
<img src="http://therouter.net/images/production/bzn/traffic_7days.png">

### Last day
<img src="http://therouter.net/images/production/bzn/traffic_last_day.png">

## Версия с LAG

### 1 day stat
<img src="http://therouter.net/images/production/bzn/the_router_bizin_monitor_lag_1.png">

### 60 days stat
<img src="http://therouter.net/images/production/bzn/the_router_bizin_monitor_lag_2.png">
