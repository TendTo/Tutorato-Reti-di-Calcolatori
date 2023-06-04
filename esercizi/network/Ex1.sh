#!/bin/bash
credentials="--username root --password root"
node1="Reti-1"
node2="Reti-2"
router="Reti-3"

echo "Configurazione nodo $node1"
vboxmanage guestcontrol "$node1" run --exe /usr/bin/hostname $credentials --wait-stdout -- hostname "$node1"
vboxmanage guestcontrol "$node1" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.1/24 dev enp0s3 
vboxmanage guestcontrol "$node1" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 10.0.1.254
echo "--------- enp0s3"
vboxmanage guestcontrol "$node1" run --exe /sbin/ip $credentials --wait-stdout -- ip addr show dev enp0s3
vboxmanage guestcontrol "$node1" run --exe /sbin/ip $credentials --wait-stdout -- ip route

# echo "--------- route table"
# echo ""
# echo "Configurazione nodo Reti-2"

# vboxmanage guestcontrol Reti-2 run --exe /usr/bin/hostname $credentials--wait-stdout -- hostname B
# vboxmanage guestcontrol Reti-2 run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.2.1/24 dev enp0s3 
# vboxmanage guestcontrol Reti-2 run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 10.0.2.2
# echo " "
# echo "Configurazione router Reti-3"

# vboxmanage guestcontrol Reti-3 run --exe /usr/bin/hostname $credentials --wait-stdout -- hostname R1
# vboxmanage guestcontrol Reti-3 run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s8 up 
# vboxmanage guestcontrol Reti-3 run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.2/24 dev enp0s3
# vboxmanage guestcontrol Reti-3 run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.2.2/24 dev enp0s8 
# vboxmanage guestcontrol Reti-3 run --exe /usr/sbin/sysctl $credentials --wait-stdout -- sysctl -w net.ipv4.ip_forward=1 

# echo " "
# echo "Test:ping A -> B"

# vboxmanage guestcontrol A run --exe /bin/ping $credentials --wait-stdout -- ping -c 4 10.0.2.1
