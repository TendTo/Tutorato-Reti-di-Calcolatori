#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
vms=(Reti-1 Reti-2 Reti-3)

# Remove all the VMs
for vm in "${vms[@]}"; do
    vboxmanage controlvm "$vm" poweroff
    sleep 1
    vboxmanage unregistervm "$vm" --delete
done

vboxmanage snapshot "$original_vm" delete "$snapshot_name"