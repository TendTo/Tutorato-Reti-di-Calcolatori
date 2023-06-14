#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
credentials="--username root --password root"
vms=("C1" "S1" "R" "C2" "S2")

# Rimuovi le VM
for vm in "${vms[@]}"; do
    echo "Removing $vm"
    vboxmanage controlvm "$vm" poweroff
    sleep 2
    vboxmanage unregistervm "$vm" --delete
done

# Rimuovi lo snapshot
vboxmanage snapshot "$original_vm" delete "$snapshot_name"