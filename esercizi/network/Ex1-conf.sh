#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
vms=(Reti-1 Reti-2 Reti-3)

vboxmanage snapshot "$original_vm" take "$snapshot_name" --description "Snapshot"

for vm in "${vms[@]}"; do
    vboxmanage clonevm "$original_vm" --snapshot "$snapshot_name" --name "$vm" --options link --register
    vboxmanage modifyvm "$vm" --memory 1024
done

vboxmanage modifyvm "${vms[0]}" --memory 1024 --nic1 intnet --intnet1 lan01 --nic2 none --nic3 none --nic4 none
vboxmanage modifyvm "${vms[1]}" --memory 1024 --nic1 intnet --intnet1 lan02 --nic2 none --nic3 none --nic4 none
vboxmanage modifyvm "${vms[2]}"  --memory 1024 --nic1 intnet --intnet1 lan01 --nic2 intnet --intnet2 lan02 --nic3 none --nic4 none

for vm in "${vms[@]}"; do
    vboxmanage startvm "$vm" --type headless
done