#!/bin/bash
original_vm="Reti"
snapshot_name="Snapshot"
vms=("A" "B" "C" "D" "E" "F" "R1" "R2" "R3" "R4")

vboxmanage snapshot "$original_vm" take "$snapshot_name" --description "Snapshot"

for vm in "${vms[@]}"; do
    vboxmanage clonevm "$original_vm" --snapshot "$snapshot_name" --name "$vm" --options link --register
    vboxmanage modifyvm "$vm" --memory 512
done

vboxmanage modifyvm "${vms[0]}" --memory 512 --nic1 intnet --intnet1 lanA
vboxmanage modifyvm "${vms[1]}" --memory 512 --nic1 intnet --intnet1 lanB
vboxmanage modifyvm "${vms[2]}" --memory 512 --nic1 intnet --intnet1 lanC
vboxmanage modifyvm "${vms[3]}" --memory 512 --nic1 intnet --intnet1 lanD
vboxmanage modifyvm "${vms[4]}" --memory 512 --nic1 intnet --intnet1 lanE
vboxmanage modifyvm "${vms[5]}" --memory 512 --nic1 intnet --intnet1 lanF

vboxmanage modifyvm "${vms[6]}" --memory 512 --nic1 intnet --intnet1 lanA --nic2 intnet --intnet2 lanB
vboxmanage modifyvm "${vms[7]}" --memory 512 --nic1 intnet --intnet1 lanB --nic2 intnet --intnet2 lanC --nic3 intnet --intnet3 lanE
vboxmanage modifyvm "${vms[8]}" --memory 512 --nic1 intnet --intnet1 lanC --nic2 intnet --intnet2 lanD
vboxmanage modifyvm "${vms[9]}" --memory 512 --nic1 intnet --intnet1 lanE --nic2 intnet --intnet2 lanF


# for vm in "${vms[@]}"; do
#     vboxmanage startvm "$vm" --type headless
# done