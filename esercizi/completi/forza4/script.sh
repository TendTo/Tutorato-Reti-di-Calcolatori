#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
credentials="--username root --password root"
local_path="/home/tend/Programming/Tutorato/Tutorato-Reti-di-Calcolatori/esercizi/completi/impiccato"
vms=("Client1" "Client2" "Server" "Router")
c_files=("client" "server")

function setup() {
    # Crea lo snapshot
    vboxmanage snapshot "$original_vm" take "$snapshot_name" --description "Snapshot"

    # Clona le VM
    for vm in "${vms[@]}"; do
        vboxmanage clonevm "$original_vm" --snapshot "$snapshot_name" --name "$vm" --options link --register
    done

    # Imposta le interfacce di rete
    vboxmanage modifyvm "${vms[0]}" --memory 512 --nic1 intnet --intnet1 lan1
    vboxmanage modifyvm "${vms[1]}" --memory 512 --nic1 intnet --intnet1 lan2
    vboxmanage modifyvm "${vms[2]}" --memory 512 --nic1 intnet --intnet1 lan3
    vboxmanage modifyvm "${vms[3]}" --memory 512 --nic1 intnet --intnet1 lan1 --nic2 intnet --intnet2 lan2 --nic3 intnet --intnet3 lan3

    # Avvia le VM
    for vm in "${vms[@]}"; do
        vboxmanage startvm "$vm"
    done

    # Aspetta che le VM siano avviate
    sleep 15

    # Configura il routing e gli indirizzi IP
    echo "Configurazione nodo ${vms[0]}"
    vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.0.1/21 dev enp0s3
    vboxmanage guestcontrol "${vms[0]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 136.16.0.254

    echo "Configurazione nodo ${vms[1]}"
    vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.8.1/21 dev enp0s3
    vboxmanage guestcontrol "${vms[1]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 136.16.8.254

    echo "Configurazione nodo ${vms[2]}"
    vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.16.1/22 dev enp0s3
    vboxmanage guestcontrol "${vms[2]}" run --exe /sbin/ip $credentials --wait-stdout -- ip route add default via 136.16.16.254

    echo "Configurazione router ${vms[3]}"
    vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s8 up
    vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip link set enp0s9 up
    vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.0.254/21 dev enp0s3
    vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.8.254/21 dev enp0s8
    vboxmanage guestcontrol "${vms[3]}" run --exe /sbin/ip $credentials --wait-stdout -- ip addr add 136.16.16.254/22 dev enp0s9
    vboxmanage guestcontrol "${vms[3]}" run --exe /usr/sbin/sysctl $credentials  --wait-stdout -- sysctl -w net.ipv4.ip_forward=1

    # Copia il file client.c e server.c in tutti i nodi
    echo "Copia dei file client.c e server.c in tutti i nodi, li compila e li rende eseguibili dall'user debian"
    for vm in "${vms[@]}"; do
        for file in "${c_files[@]}"; do
            vboxmanage guestcontrol $vm copyto "$local_path/$file.c" "/home/debian/$file.c" $credentials
            vboxmanage guestcontrol $vm run --exe /usr/bin/gcc $credentials --wait-stdout -- gcc "/home/debian/$file.c" -o "/home/debian/$file"
            vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian "/home/debian/$file.c"
            vboxmanage guestcontrol $vm run --exe /bin/chown $credentials --wait-stdout -- chown debian:debian "/home/debian/$file"
        done
    done
}

function cleanup() {
    # Rimuovi le VM
    for vm in "${vms[@]}"; do
        echo "Removing $vm"
        vboxmanage controlvm "$vm" poweroff
        sleep 2
        vboxmanage unregistervm "$vm" --delete
    done

    # Rimuovi lo snapshot
    vboxmanage snapshot "$original_vm" delete "$snapshot_name"
}

case "$1" in
    setup)
        setup
        ;;
    cleanup)
        cleanup
        ;;
    *)
        echo "Usage: $0 <setup|cleanup>"
        exit 1
esac
