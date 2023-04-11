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

### Connessione ssh

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

<!-- New subsection -->