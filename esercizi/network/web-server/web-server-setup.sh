#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
credentials="--username root --password root"
vms=("client" "server" "database" "routerISP" "routerInterno")
c_files=("database" "server") # no .c extension

# Crea lo snapshot
vboxmanage snapshot "$original_vm" take "$snapshot_name" --description "Snapshot"

# Clona le VM
for vm in "${vms[@]}"; do
    vboxmanage clonevm "$original_vm" --snapshot "$snapshot_name" --name "$vm" --options link --register
    vboxmanage modifyvm "$vm" --memory 512
done

# Imposta le interfacce di rete
# client
vboxmanage modifyvm "${vms[0]}" --nic1 intnet --intnet1 internet
# server
vboxmanage modifyvm "${vms[1]}" --nic1 intnet --intnet1 lan1
# database
vboxmanage modifyvm "${vms[2]}" --nic1 intnet --intnet1 lan2
# routerISP
vboxmanage modifyvm "${vms[3]}" --nic1 intnet --intnet1 internet --nic2 intnet --intnet2 lan1
# routerInterno
vboxmanage modifyvm "${vms[4]}" --nic1 intnet --intnet1 lan1 --nic2 intnet --intnet2 lan2

# Avvia le VM
for vm in "${vms[@]}"; do
    vboxmanage startvm "$vm"
done

# Aspetta che le VM siano avviate
sleep 20

# Configura il routing e gli indirizzi IP
echo "Configurazione nodo ${vms[0]}"
vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 192.168.1.1/24 dev enp0s3
vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 192.168.1.254

echo "Configurazione nodo ${vms[1]}"
vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.1/24 dev enp0s3
vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 10.0.1.254
vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add 172.16.0.0/16 via 10.0.1.253

echo "Configurazione nodo ${vms[2]}"
vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 172.16.2.2/16 dev enp0s3
vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 172.16.1.253

echo "Configurazione nodo ${vms[3]}"
vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s8 up
vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 192.168.1.254/24 dev enp0s3
vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.254/24 dev enp0s8
vboxmanage guestcontrol "${vms[3]}" run --exe /usr/sbin/sysctl $credentials  --wait-stdout -- sysctl -w net.ipv4.ip_forward=1

echo "Configurazione nodo ${vms[4]}"
vboxmanage guestcontrol "${vms[4]}" run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s8 up
vboxmanage guestcontrol "${vms[4]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 10.0.1.253/24 dev enp0s3
vboxmanage guestcontrol "${vms[4]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 172.16.1.253/16 dev enp0s8
vboxmanage guestcontrol "${vms[4]}" run --exe /usr/sbin/sysctl $credentials  --wait-stdout -- sysctl -w net.ipv4.ip_forward=1

echo "Aggiungi le entry DNS"
for vm in "${vms[@]}"; do
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '$vm' >> /etc/hostname"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '127.0.0.1 $vm' >> /etc/hosts"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '192.168.1.1 client' >> /etc/hosts"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '192.168.1.254 routerISP' >> /etc/hosts"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '10.0.1.1 server' >> /etc/hosts"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '10.0.1.253 routerInterno' >> /etc/hosts"
    vboxmanage guestcontrol "$vm" run --exe /bin/sh $credentials --wait-stdout -- sh -c "echo '172.16.2.2 database' >> /etc/hosts"
done

# Copia il file client.c e server.c in tutti i nodi
echo "Copia dei file client.c e server.c in tutti i nodi, li compila e li rende eseguibili dall'user debian"
for vm in "${vms[@]}"; do
    for file in "${c_files[@]}"; do
        vboxmanage guestcontrol $vm copyto "/home/tend/Programming/Tutorato/Tutorato-Reti-di-Calcolatori/esercizi/socket/web-server/$file.c" "/home/debian/$file.c" $credentials
        vboxmanage guestcontrol $vm run --exe /usr/bin/gcc $credentials --wait-stdout -- gcc "/home/debian/$file.c" -o "/home/debian/$file"
        vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian "/home/debian/$file.c"
        vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian "/home/debian/$file"
    done
done
