@echo off

SET "original_vm=Reti"
SET "snapshot_name=Snapshot"

VBoxManage.exe "controlvm" "A" "poweroff"
VBoxManage.exe "controlvm" "B" "poweroff"
VBoxManage.exe "controlvm" "R1" "poweroff"
sleep "1"
VBoxManage.exe "unregistervm" "A" "--delete"
VBoxManage.exe "unregistervm" "B" "--delete"
VBoxManage.exe "unregistervm" "R1" "--delete"

VBoxManage.exe "snapshot" "%original_vm%" "delete" "%snapshot_name%"