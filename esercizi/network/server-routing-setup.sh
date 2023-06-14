#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
credentials="--username root --password root"
vms=("C1" "S1" "R" "C2" "S2")

# Crea lo snapshot
vboxmanage snapshot "$original_vm" take "$snapshot_name" --description "Snapshot"

# Clona le VM
for vm in "${vms[@]}"; do
    vboxmanage clonevm "$original_vm" --snapshot "$snapshot_name" --name "$vm" --options link --register
done

# Imposta le interfacce di rete
vboxmanage modifyvm "${vms[0]}" --memory 512 --nic1 intnet --intnet1 lanA --nic2 hostonly --hostonlyadapter2 vboxnet0
vboxmanage modifyvm "${vms[1]}" --memory 512 --nic1 intnet --intnet1 lanA --nic2 hostonly --hostonlyadapter2 vboxnet0
vboxmanage modifyvm "${vms[2]}" --memory 512 --nic1 intnet --intnet1 lanA --nic2 intnet --intnet2 lanB
vboxmanage modifyvm "${vms[3]}" --memory 512 --nic1 intnet --intnet1 lanB --nic2 hostonly --hostonlyadapter2 vboxnet0
vboxmanage modifyvm "${vms[4]}" --memory 512 --nic1 intnet --intnet1 lanB --nic2 hostonly --hostonlyadapter2 vboxnet0

# Avvia le VM
for vm in "${vms[@]}"; do
    vboxmanage startvm "$vm"
done

# Aspetta che le VM siano avviate
sleep 16

# Configura il routing e gli indirizzi IP
echo "Configurazione nodo ${vms[0]}"
vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.2/24 dev enp0s3
vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 10.0.1.254

echo "Configurazione nodo ${vms[1]}"
vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.1/24 dev enp0s3
vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 10.0.1.254

echo "Configurazione router ${vms[2]}"
vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s8 up
vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.254/24 dev enp0s3
vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 192.168.1.254/24 dev enp0s8
vboxmanage guestcontrol "${vms[2]}" run --exe /usr/sbin/sysctl $credentials  --wait-stdout -- sysctl -w net.ipv4.ip_forward=1

echo "Configurazione nodo ${vms[3]}"
vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 192.168.1.2/24 dev enp0s3
vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 192.168.1.254

echo "Configurazione nodo ${vms[4]}"
vboxmanage guestcontrol "${vms[4]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 192.168.1.1/24 dev enp0s3
vboxmanage guestcontrol "${vms[4]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 192.168.1.254

# Copia il file client.c e server.c in tutti i nodi
echo "Copia dei file client.c e server.c in tutti i nodi, li compila e li rende eseguibili dall'user debian"
for vm in "${vms[@]}"; do
    vboxmanage guestcontrol $vm copyto '/home/tend/Programming/Tutorato/Tutorato-Reti-di-Calcolatori/esercizi/socket/server-routing/client.c' '/home/debian/client.c' $credentials
    vboxmanage guestcontrol $vm copyto '/home/tend/Programming/Tutorato/Tutorato-Reti-di-Calcolatori/esercizi/socket/server-routing/server.c' '/home/debian/server.c' $credentials
    vboxmanage guestcontrol $vm run --exe /usr/bin/gcc $credentials --wait-stdout -- gcc /home/debian/client.c -o /home/debian/client
    vboxmanage guestcontrol $vm run --exe /usr/bin/gcc $credentials --wait-stdout -- gcc /home/debian/server.c -o /home/debian/server
    vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian /home/debian/client
    vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian /home/debian/server
done