# Esercizio Tris

## Definizione dello scenario

Siano date tre macchine: **Server**, **Client1** e **Client2**.  
Ciascuna delle macchine appartiene ad una sotto-rete separata.
Le comunicazione fra di loro avviene per mezzo di un **Router**, connesso a tutte e tre le LAN.

### Progettazione della topologia

Dato un range di indirizzi identificato da $172.0.0.0/20$, assegnare gli indirizzi alle sotto-reti LanA, LanB, LanC in maniera tale che

- LanA sia in grado di ospitare fino a 400 Hosts.
  - Il **Server** apparterrà a questa sotto-rete
- LanB sia in grado di ospitare fino a 200 Hosts.
  - Il **Client1** apparterrà a questa sotto-rete
- LanC sia in grado di ospitare fino a 200 Hosts.
  - Il **Client2** apparterrà a questa sotto-rete

Si ha completa libertà nell'assegnamento degli ip alle macchine in ogni sotto-rete, purché tale ip sia valido.

```mermaid
flowchart TB


r{{router}}

subgraph lA[LanA - 400 Hosts]
    s[server]
end

subgraph lB[<tab>LanB - 200 Hosts]
    c1[client1]
end

subgraph lC[LanC - 200 Hosts]
    c2[client2]
end

    r --- lA & lB & lC
```

[Soluzione](#soluzione-progettazione-della-topologia)

### Realizzazione del networking

Dopo aver abbozzato la topologia su carta, si realizzi il networking definito in precedenza con delle macchine virtuali.  
Le macchine devono essere in grado di pingare tutte le altre.

[Soluzione](#soluzione-realizzazione-del-networking)

### Programmazione Socket

Realizzare un programma scritto in c (o c++) che simuli una partita a "Tris" fra i due **Client**, con il **Server** in mezzo a fare da arbitro.
Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.  
Appena avviato, ogni **Client** si connette al **Server** per registrarsi, fornendo anche le informazioni che il **Server** userà per comunicare durante la partita (ip, porta).  
Quando entrambi i **Client** si sono registrati, il **Server** avvia la partita.  
Inizia quindi ad interpellare a turno i due giocatori, chiedendo dove porre il loro simbolo sulla griglia.
Dopo ogni mossa, il **Server** risponde inviando la griglia aggiornata.  
Quando un giocatore ha vinto, il **Server** invia un messaggio di vittoria al vincitore e di sconfitta al perdente, quindi termina la partita.

```mermaid
sequenceDiagram
actor c1 as Client1
participant s as Server
actor c2 as Client2

c1 ->> s: register
c2 ->> s: register
loop while game is not over
    s ->>+ c1: your turn
    c1 -->>- s: move
    s -->> c1: grid
    s ->>+ c2: your turn
    c2 -->>- s: move
    s -->> c2: grid
end
s ->> c1: game result
s ->> c2: game result
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
r{{rete\n172.0.0.0/20\n4k hosts}}
s1{{sotto-rete\n172.0.0.0/21\n2k hosts}}
s2{{sotto-rete\n172.0.8.0/21\n2k hosts}}
s3{{sotto-rete\n172.0.0.0/22\n1022 hosts}}
s4{{sotto-rete\n172.0.4.0/22\n1022 hosts}}
s5{{sotto-rete\n172.0.0.0/23\n510 hosts}}
s6{{sotto-rete\n172.0.2.0/23\n510 hosts}}
s7{{sotto-rete\n172.0.0.0/24\n254 hosts}}
s8{{sotto-rete\n172.0.1.0/24\n254 hosts}}


style s6 stroke:#f00,stroke-width:4px
style s7 stroke:#0f0,stroke-width:4px
style s8 stroke:#00f,stroke-width:4px

    r -- 0 --> s1
    r -- 1 --> s2
    s1 -- 0 --> s3
    s1 -- 1 --> s4
    s3 -- 0 --> s5
    s3 -- 1 --> s6
    s5 -- 0 --> s7
    s5 -- 1 --> s8
```

---

```mermaid
flowchart TB


r{{router}}

subgraph lA[LanA 172.0.2.0/23]
    s[server\n172.0.2.1]
end

subgraph lB[LanB 172.0.0.0/24]
    c1[client1\n172.0.0.1]
end

subgraph lC[LanC 172.0.1.0/24]
    c2[client2\n172.0.1.1]
end

style lA stroke:#f00
style lB stroke:#0f0
style lC stroke:#00f

    r --172.0.2.254--- lA
    r --172.0.0.254--- lB
    r --172.0.1.126--- lC
```

### Soluzione: Realizzazione del networking

#### Server

```shell
# Server
ip addr add 172.0.2.1/23
ip route add default via 172.0.2.254
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 172.0.2.1/23
    gateway 172.0.2.254
```

#### Client 1

```shell
# Client1
ip addr add 172.0.0.1/24
ip route add default via 172.0.0.254
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 172.0.0.1/24
    gateway 172.0.0.254

```

#### Client2

```shell
# Client2
ip addr add 172.0.1.1/24
ip route add default via 172.0.1.254
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 172.0.1.1/24
    gateway 172.0.1.254
```

#### Router

```shell
# Router
ip link set enp0s8 up
ip link set enp0s9 up
ip addr add 172.0.2.254/23 dev enp0s3
ip addr add 172.0.0.254/24 dev enp0s8
ip addr add 172.0.1.254/24 dev enp0s9
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 172.0.2.254/23

auto enp0s8
iface enp0s8 inet static
    address 172.0.0.254/24

auto enp0s9
iface enp0s9 inet static
    address 172.0.1.254/24
```

```py
# Router
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

### Soluzione: Programmazione Socket

[server.c](./server.c)  
[client.c](./client.c)
