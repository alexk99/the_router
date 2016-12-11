# Dynamic routing. Integration with Quagga routing suite

Integration with Quagga is built on using Kernel network interfaces (KNI) on the DPDK side 
and <a href="http://www.nongnu.org/quagga/docs/docs-multi/zebra-FIB-push-interface.html">zebra FIB push interface</a> 
on the quagga's side.

<img src="http://therouter.net/images/quagga.png">

KNI interfaces are created for each router's virtual interface (VIF) that are going to participate in
the dynamic routing interaction with external routers. To do so 'kni' flag must be used in the creation
of a VIF. Kni flag instructs the router's core to create KNI interface in the linux kernel and forward to it
all packets that are destined to any ip address of the parent VIF. In other words all the conroll plane traffic
go through the KNI interfaces, so Quagga BGP daemon can receive it and send responses back to the world.

Once the quagga has received a route it will try to push it throuht its 'FIB push inteface'. The router listens
on that interdface for route updates add install them into its main routing table. That updates
will instruct router's data plane core to forward traffic to the right destinaion. So, control plane trafic
goes along slow path through KNI to the quagga and then back to the router through FPM interface. But data traffic
will always go along fast path right through routers core to the destination.

## Configuration example
