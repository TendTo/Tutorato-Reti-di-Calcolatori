# Impiccato

## Definizione dello scenario

Gli studenti del corso di Reti di Calcolatori del DMI - UNICT hanno deciso, di loro spontanea volontà, di realizzare il gioco dell'impiccato tra macchine virtuali.

### Progettazione della topologia

Configurare la rete secondo lo schema riportato in figura. Gli indirizzi ip da utilizzare sono nel range $149.54.0.0/16$

- Lan1 sia in grado di ospitare fino a 1000 Hosts.
  - Il **Client1** apparterrà a questa sotto-rete
- Lan2 sia in grado di ospitare fino a 1000 Hosts.
  - Il **Client2** apparterrà a questa sotto-rete
- Lan3 sia in grado di ospitare fino a 512 Hosts.
  - Il **Server** apparterrà a questa sotto-rete

Si ha completa libertà nell'assegnamento degli ip alle macchine in ogni sotto-rete, purché tale ip sia valido.

```mermaid
flowchart TB


r{{router}}

subgraph l1[Lan1 - 1000 Hosts]
    c1[Client 1]
end

subgraph l2[Lan2 - 1000 Hosts]
    c2[Client 2]
end

subgraph l3[Lan3 - 512 Hosts]
    s[Server]
end

    r --- l1 & l2 & l3
```

[Soluzione](#soluzione-progettazione-della-topologia)

### Realizzazione del networking

Dopo aver abbozzato la topologia su carta, si realizzi il networking definito in precedenza con delle macchine virtuali.  
Le macchine devono essere in grado di pingare tutte le altre.

[Soluzione](#soluzione-realizzazione-del-networking)

### Programmazione Socket

Due macchine client si sfideranno tra di loro a fine di indovinare la parola segreta presente in una macchina server.
Ogni macchina client avrà a disposizione 3 vite. Il conteggio delle vite sarà tenuto dal server.
Ad ogni turno:

- Il server prima invierà al client di turno lo stato del gioco: lettere indovinate, lettere mancanti e struttura della parola
- Ogni client ad ogni turno avrà la possibilità solo di scrivere da riga di comando o una lettere o dare la risposta completa.
  In caso di errore, perderà una vita.
  Il server notificherà allo specifico client l'esito del turno.
- Se il giocatore di turno perde tutte le vite, dovrà terminare l'esecuzione del programma.

Il server dovrà memorizzare in una struct client il nome del giocatore, l'indirizzo IP e la porta del client.
Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.  
Il server sceglierà randomicamente una stringa da utilizzare per il gioco dell'impiccato tra quelle presenti in un array bidimensionale.  
Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).

Progettare ed implementare **L'impiccato!** tramite socket UDP, considerando l'architettura vista prima.

```mermaid
sequenceDiagram
actor c1 as Client1
participant s as Server
actor c2 as Client2

c1 ->> s: register
c2 ->> s: register
loop while c1 lives > 0 or c2 lives > 0
    s ->>+ c1: game state
    c1 -->>- s: move
    s ->>+ c2: game state
    c2 -->>- s: move

    alt c1 lives == 0
        s ->> c1: game over
        c1 -x c1: exit
    else c2 lives == 0
        s ->> c2: game over
        c2 -x c2: exit
    else word guessed by c1
        s ->> c1: won
        c1 -x c1: exit
        s ->> c2: game over
        c2 -x c2: exit
        s -x s: exit
    else word guessed by c2
        s ->> c1: game over
        c1 -x c1: exit
        s ->> c2: win
        c2 -x c2: exit
        s -x s: exit
    end
end
```

[Soluzione](#soluzione-programmazione-socket)

---

## Soluzioni

> Prima di leggere le soluzioni, provare a risolvere l'esercizio da soli.
> Dopo averlo fatto, confrontare la propria soluzione con quella proposta.
> Ci sono tantissimi modi per risolvere le varie consegne, quindi non c'è da preoccuparsi se la propria soluzione è diversa da quella proposta.

### Soluzione: Progettazione della topologia

```mermaid
graph TD
r{{rete\n149.54.0.0/16\n64k hosts}}
s1{{sotto-rete\n149.54.0.0/17\n32k hosts}}
s2{{sotto-rete\n149.54.128.0/17\n32k hosts}}
s3{{sotto-rete\n149.54.0.0/18\n16k hosts}}
s4{{sotto-rete\n149.54.64.0/18\n16k hosts}}
s5{{sotto-rete\n149.54.0.0/19\n8k hosts}}
s6{{sotto-rete\n149.54.32.0/19\n8k hosts}}
s7{{sotto-rete\n149.54.0.0/20\n4k hosts}}
s8{{sotto-rete\n149.54.16.0/20\n4k hosts}}
s9{{sotto-rete\n149.54.0.0/21\n2k hosts}}
s10{{sotto-rete\n149.54.8.0/21\n2k hosts}}
s11{{sotto-rete\n149.54.0.0/22\n1022 hosts}}
s12{{sotto-rete\n149.54.4.0/22\n1022 hosts}}
s13{{sotto-rete\n149.54.8.0/22\n1022 hosts}}
s14{{sotto-rete\n149.54.12.0/22\n1022 hosts}}


style s11 stroke:#f00,stroke-width:4px
style s12 stroke:#0f0,stroke-width:4px
style s13 stroke:#00f,stroke-width:4px

    r -- 0 --> s1
    r -- 1 --> s2
    s1 -- 0 --> s3
    s1 -- 1 --> s4
    s3 -- 0 --> s5
    s3 -- 1 --> s6
    s5 -- 0 --> s7
    s5 -- 1 --> s8
    s7 -- 0 --> s9
    s7 -- 1 --> s10
    s9 -- 0 --> s11
    s9 -- 1 --> s12
    s10 -- 0 --> s13
    s10 -- 0 --> s14
```

---

```mermaid
flowchart TB


r{{router}}

subgraph l1[Lan1 149.54.0.0/22]
    c1[Client 1\n149.54.0.1]
end

subgraph l2[Lan2 149.54.4.0/23]
    c2[Client 2\n149.54.4.1]
end

subgraph l3[Lan3 149.54.8.0/25]
    s[Server\n149.54.8.1]
end

style l1 stroke:#f00
style l2 stroke:#0f0
style l3 stroke:#00f

    r --149.54.0.254--- l1
    r --149.54.4.254--- l2
    r --149.54.8.254--- l3
```

### Soluzione: Realizzazione del networking

#### Client 1

```shell
# Client1
ip addr add 149.54.0.1/22
ip route add default via 149.54.0.254
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 149.54.0.1/22
    gateway 149.54.0.254

```

#### Client2

```shell
# Client2
ip addr add 149.54.4.1/22
ip route add default via 149.54.4.254
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 149.54.4.1/22
    gateway 149.54.4.254
```

#### Server

```shell
# Server
ip addr add 149.54.8.1/22
ip route add default via 149.54.8.254
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 149.54.8.1/22
    gateway 149.54.8.254
```

#### Router

```shell
# Router
ip link set enp0s8 up
ip link set enp0s9 up
ip addr add 149.54.0.254/22 dev enp0s3
ip addr add 149.54.4.254/22 dev enp0s8
ip addr add 149.54.8.254/22 dev enp0s9
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 149.54.0.254/22

auto enp0s8
iface enp0s8 inet static
    address 149.54.4.254/22

auto enp0s9
iface enp0s9 inet static
    address 149.54.8.254/22
```

```py
# Router
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

### Soluzione: Programmazione Socket

[server.c](./server.c)  
[client.c](./client.c)
