@echo off

SET "credentials=--username root --password root"
SET "node1=Reti-1"
SET "node2=Reti-2"
SET "router=Reti-3"

echo "Configurazione nodo %node1%"
VBoxManage.exe "guestcontrol" "%node1%" "run" "--exe" "\usr\bin\hostname" "%credentials%" "--wait-stdout" "--" "hostname" "%node1%"
VBoxManage.exe "guestcontrol" "%node1%" "run" "--exe" "\sbin\ip" "%credentials%" "--wait-stdout" "--" "ip" "addr" "add" "10.0.1.1\24" "dev" "enp0s3"
VBoxManage.exe "guestcontrol" "%node1%" "run" "--exe" "\sbin\ip" "%credentials%" "--wait-stdout" "--" "ip" "route" "add" "default" "via" "10.0.1.254"
echo "--------- enp0s3"
VBoxManage.exe "guestcontrol" "%node1%" "run" "--exe" "\sbin\ip" "%credentials%" "--wait-stdout" "--" "ip" "addr" "show" "dev" "enp0s3"
VBoxManage.exe "guestcontrol" "%node1%" "run" "--exe" "\sbin\ip" "%credentials%" "--wait-stdout" "--" "ip" "route"
