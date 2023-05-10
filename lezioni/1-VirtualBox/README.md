# VirtualBox

Utilizzo del software di virtualizzazione VirtualBox.

<!-- New section -->

## Cosa sono le macchine virtuali?

![Macchine virtuali](./img/VM.svg)

Le macchine virtuali sono software che permettono di emulare un intero computer.

<!-- New subsection -->

### Benefici della virtualizzazione

- Eseguire più sistemi diversi contemporaneamente\*
- Semplificare l'installazione ed utilizzo di software specifici
- Disponibilità e disaster recovery
- Debuggare e testare configurazioni di rete o architetture distribuite
- Cloud

<!-- New section -->

## Cos'è VirtualBox?

<img src="./img/virtualbox_logo.png" width="300px"/></img>

VirtualBox è un software di virtualizzazione, ovvero un programma che permette di creare e gestire macchine virtuali.

<!-- New subsection -->

### Terminologia

- **Host Operating System (host OS)**: sistema operativo che esegue VirtualBox
- **Guest Operating System (guest OS)**: sistema operativo che gira all'interno della macchina virtuale
- **Macchina virtuale (VM)**: ambiente creato da VirtualBox che emula un computer e le sue componenti
- **Guest Additions**: software opzionale che aggiunge diverse funzionalità alle VM create con VirtualBox

<!-- New subsection -->

### Immagini

I sistemi operativi vengono spesso distribuito in formato **iso**, ovvero un file immagine che contiene i file necessari per la sua istallazione.

Alternativamente, dopo aver creato una prima VM, questa può essere clonata per creare altre VM identiche con lo stesso stato.

<!-- .element: class="fragment" -->

<!-- New section -->

## Creare una macchina virtuale

Vediamo i passi necessari per creare una macchina virtuale.

<!-- New subsection -->

### Immagine del sistema operativo

Scarichiamo l'immagine del sistema operativo che vogliamo installare.

In questo caso, utilizzeremo [debian](https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/).

<!-- .element: class="fragment" -->

La guida riguardo la configurazione di VirtualBox può essere consultata sul [sito](https://www.virtualbox.org/manual/UserManual.html#create-vm-wizard).

<!-- .element: class="fragment" -->

Nel momento in cui viene avviata, selezionando l'installazione grafica sarà sufficiente seguire le istruzioni a schermo.

<!-- .element: class="fragment" -->

<!-- New section -->

## Connessione ssh

Per utilizzare più agevolmente la VM, può essere comodo connettersi ad essa tramite ssh.

Questo può essere fatto dopo aver attivato il server ssh sul guest OS.  
Se l'installazione è stata quella guidata, dovrebbe essere già attivo.
Altrimenti è necessario seguire [questi passi](https://wiki.debian.org/it/SSH#Installazione_del_server).

<!-- .element: class="fragment" -->

<!-- New subsection -->

### Port forwarding

Di default, VirtualBox crea una connessione di tipo NAT.

Per poterci connettere alla VM, dobbiamo effettuare un port forwarding.

<!-- .element: class="fragment" data-fragment-index="1" -->

<img src="./img/port_forwarding.png" width="850px"/></img>

<!-- .element: class="fragment" data-fragment-index="1" -->

<!-- New subsection -->

### Connessione ssh con NAT

Dall'host OS, possiamo connetterci alla VM tramite ssh.

```shell
ssh -p 2222 debian@localhost
```

<!-- New section -->

## Tipi di rete

VirtualBox supporta diversi tipi di rete.  
La scelta influenza la raggiungibilità delle VM e la loro connessione ad altre reti.

[Ulteriori dettagli](https://www.nakivo.com/blog/virtualbox-network-setting-guide/)  
[Ulteriori dettagli](https://www.virtualbox.org/manual/ch06.html)

<!-- New subsection -->

### Confronto

|    Mode     | VM → Host | VM ← Host  | VM1 ↔ VM2 | VM → LAN |  VM ← LAN  |
| :---------: | :-------: | :--------: | :-------: | :------: | :--------: |
|   Bridged   |     ✓     |     ✓      |     ✓     |    ✓     |     ✓      |
| NAT network |     ✓     | Forwarding |     ✓     |    ✓     | Forwarding |
|     NAT     |     ✓     | Forwarding |           |    ✓     | Forwarding |
|  Host-only  |     ✓     |     ✓      |     ✓     |          |            |
|  Internal   |           |            |     ✓     |          |            |
|  Detached   |           |            |           |          |            |

<!-- New subsection -->

### NAT

<img src="./img/NAT_mode.webp" width="80%"/></img>

<!-- New subsection -->

### NAT network

<img src="./img/NAT_network_mode.webp" width="80%"/></img>

<!-- New subsection -->

### Bridged

<img src="./img/bridged_mode.webp" width="80%"/></img>

<!-- New subsection -->

### Internal network

<img src="./img/internal_mode.webp" width="80%"/></img>

<!-- New subsection -->

### Host only

<img src="./img/host_only_mode.webp" width="80%"/></img>

<!-- New section -->

## Passaggi chiave

Di seguito ci sarà una serie di passaggi o di controlli da fare per verificare la correttezza della configurazione di VirtualBox.

Assicuratevi che il passaggio sia fatto sulla macchina corretta guardando il prompt della shell: host (il vostro computer) o vm (la macchina virtuale).

<!-- New subsection -->

### Rete host-only

Dopo aver effettuato tutti gli eventuali aggiornamenti ed installazioni che necessitano internet, generalmente si userà la rete **host-only**.

Bisogna assicurarsi che Virtualbox abbia creato tale rete.

![Host-only network](./img/host_only_setting.jpeg)

<!-- New subsection -->

### Interfaccia di rete host-only

A quel punto, sulla VM, si deve impostare l'interfaccia di rete corretta.
Il nome deve essere lo stesso della rete creata in precedenza.

Se state facendo il setup di una macchina clonata, assicuratevi di cambiare il MAC address, altrimenti riceverà lo stesso indirizzo ip dell'originale.

<!-- .element: class="fragment" -->

![Host-only network interface](./img/host_only_vm.png)

<!-- .element: class="fragment" -->

<!-- New subsection -->

### Conoscere l'indirizzo ip della VM

Per poter connettere la VM, è necessario conoscere il suo indirizzo ip.
Questo può essere fatto con il comando `ip a`.

```shell
user@vm:~$ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue ...
    ...
2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 ...
    link/ether 0a:00:27:00:00:00 brd ff:ff:ff:ff:ff:ff
    inet 192.168.56.24/24 brd 192.168.56.255 scope global enp0s3
       valid_lft forever preferred_lft forever
    inet6 fe80::800:27ff:fe00:3754/64 scope link
       valid_lft forever preferred_lft forever
```

L'interfaccia è la `enp0s3`, e gli indirizzi sono `192.168.56.24` per IpV4 e `fe80::800:27ff:fe00:2754` per IpV6.

<!-- .element: class="fragment" -->

<!-- New subsection -->

#### Check della connessione

A questo punto la VM dovrebbe essere raggiungibile dall'host.
La connessione può essere verificata con il comando `ping`.

```shell
# ping <ip VM>
user@host:~$ ping 192.168.56.24
```

<!-- New subsection -->

### SSH

SSH è un tool, nonché un protocollo, per poter accedere alla VM da remoto.
La VM deve fornire un server SSH a cui l'host si connetterà.

Per verificare che il server SSH sia attivo, si può usare il comando `systemctl`.
Potrebbe essere necessario essere root.

<!-- .element: class="fragment" data-fragment-index="1" -->

```shell
root@vm:~$ systemctl status ssh
ssh.service - OpenBSD Secure Shell server
    Loaded: loaded (/lib/systemd/system/ssh.service; enabled)
    Active: active (running) since Fri 2021-09-24 15:27:55 CEST; 1h 26min ago
    ...
```

<!-- .element: class="fragment" data-fragment-index="1" -->

<!-- New subsection -->

#### Installazione server SSH

Se il server SSH non è installato, può essere fatto con il comando `apt`.
Bisogna assicurarsi di avere i privilegi di root.  
Inoltre sarà necessario accedere ad internet per effettuare il download.
Quindi la VM dovrà essere connessa utilizzando la rete NAT.

```shell
root@vm:~$ apt update # Aggiornamento dei pacchetti, se non ancora fatto
root@vm:~$ apt install openssh-server
```

<!-- New subsection -->

#### Check della connessione ssh

A questo punto la VM dovrebbe essere raggiungibile dall'host tramite ssh.
La connessione può essere verificata con il comando `ssh`.

```shell
# ssh <user>@<ip VM>
user@host:~$ ssh user@192.168.56.24
```

La prima volta che effettuate la connessione vi verrà chiesto di accettare la chiave pubblica (fingerprint) del server ssh.
Per proseguire basta digitare `yes`.

<!-- .element: class="fragment" -->

<!-- New subsection -->

### Connessione con VsCode remote ssh

VsCode offre l'estensione [Remote - SSH](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-ssh) che fornisce una connessione ssh agevolata.

[Ulteriori dettagli](https://code.visualstudio.com/docs/remote/ssh-tutorial)
