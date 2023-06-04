@echo off

SET "original_vm=Reti"
SET "snapshot_name=Snapshot"

VBoxManage.exe "snapshot" "%original_vm%" "take" "%snapshot_name%" "--description" "Snapshot"

VBoxManage.exe "clonevm" "%original_vm%" "--snapshot" "%snapshot_name%" "--name" "A" "--options" "link" "--register"
VBoxManage.exe "clonevm" "%original_vm%" "--snapshot" "%snapshot_name%" "--name" "B" "--options" "link" "--register"
VBoxManage.exe "clonevm" "%original_vm%" "--snapshot" "%snapshot_name%" "--name" "R1" "--options" "link" "--register"

VBoxManage.exe "modifyvm" "A" "--memory" "1024" "--nic1" "intnet" "--intnet1" "lan01" "--nic2" "none" "--nic3" "none" "--nic4" "none"
VBoxManage.exe "modifyvm" "B" "--memory" "1024" "--nic1" "intnet" "--intnet1" "lan02" "--nic2" "none" "--nic3" "none" "--nic4" "none"
VBoxManage.exe "modifyvm" "R1" "--memory" "1024" "--nic1" "intnet" "--intnet1" "lan01" "--nic2" "intnet" "--intnet2" "lan02" "--nic3" "none" "--nic4" "none"

VBoxManage.exe "startvm" "A" "--type" "headless"
VBoxManage.exe "startvm" "B" "--type" "headless"
VBoxManage.exe "startvm" "R1" "--type" "headless"