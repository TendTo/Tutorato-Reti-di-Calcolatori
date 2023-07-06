# Esercizio Rock Paper Scissors

## Definizione dello scenario

Siano date tre macchine: **Server**, **Client1** e **Client2**.  
Ciascuna delle macchine appartiene ad una sotto-rete separata.
Le comunicazione fra di loro avviene per mezzo di un **Router**, connesso a tutte e tre le LAN.

### Progettazione della topologia

Dato un range di indirizzi identificato da $10.0.0.0/16$, assegnare gli indirizzi alle sotto-reti LanA, LanB, LanC in maniera tale che

- LanA sia in grado di ospitare fino a 1000 Hosts.
  - Il **Server** apparterrà a questa sotto-rete
- LanB sia in grado di ospitare fino a 500 Hosts.
  - Il **Client1** apparterrà a questa sotto-rete
- LanC sia in grado di ospitare fino a 100 Hosts.
  - Il **Client2** apparterrà a questa sotto-rete

Si ha completa libertà nell'assegnamento degli ip alle macchine in ogni sotto-rete, purché tale ip sia valido.

```mermaid
flowchart TB


r{{router}}

subgraph lA[LanA - 1000 Hosts]
    s[server]
end

subgraph lB[<tab>LanB - 500 Hosts]
    c1[client1]
end

subgraph lC[LanC - 100 Hosts]
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

Realizzare un programma scritto in c (o c++) che simuli una partita a "Carta-Sasso-Forbice" fra i due **Client**, con il **Server** in mezzo a fare da arbitro.
Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.  
Appena avviato, ogni **Client** si connette al **Server** per registrarsi, fornendo anche le informazioni che il **Server** userà per comunicare durante la partita (ip, porta).  
Quando entrambi i **Client** si sono registrati, il **Server** avvia la partita, impostando il numero di vite di entrambi i giocatori ad un valore stabilito.
Inizia quindi ad interpellare a turno i due giocatori, chiedendo la loro mossa.
Quando entrambi hanno giocato, il **Server** comunica il risultato della partita a entrambi i giocatori.
Se non si è verificato un pareggio, toglie una vita a quello che ha perso e ricomincia il ciclo.  
Quando un giocatore rimane senza vite, il **Server** comunica il vincitore ad entrambi e termina la partita.

```mermaid
sequenceDiagram
actor c1 as Client1
participant s as Server
actor c2 as Client2

c1 ->>+ s: register
c2 ->>+ s: register
loop while c1 lives > 0 and c2 lives > 0
    s ->>+ c1: your turn
    c1 -->>- s: move
    s ->>+ c2: your turn
    c2 -->>- s: move
alt if c1 wins
    s ->> s: remove life to c2
else if c2 wins
    s ->> s: remove life to c1
end
s ->> c1: round result
s ->> c2: round result
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
r{{rete\n10.0.0.0/16\n64k hosts}}
s1{{sotto-rete\n10.0.0.0/17\n32k hosts}}
s2{{sotto-rete\n10.0.128.0/17\n32k hosts}}
s3{{sotto-rete\n10.0.0.0/18\n16k hosts}}
s4{{sotto-rete\n10.0.64.0/18\n16k hosts}}
s5{{sotto-rete\n10.0.0.0/19\n8k hosts}}
s6{{sotto-rete\n10.0.32.0/19\n8k hosts}}
s7{{sotto-rete\n10.0.0.0/20\n4k hosts}}
s8{{sotto-rete\n10.0.16.0/20\n4k hosts}}
s9{{sotto-rete\n10.0.0.0/21\n2k hosts}}
s10{{sotto-rete\n10.0.8.0/21\n2k hosts}}
s11{{sotto-rete\n10.0.0.0/22\n1022 hosts}}
s12{{sotto-rete\n10.0.4.0/22\n1022 hosts}}
s13{{sotto-rete\n10.0.0.0/23\n510 hosts}}
s14{{sotto-rete\n10.0.2.0/23\n510 hosts}}
s15{{sotto-rete\n10.0.0.0/24\n254 hosts}}
s16{{sotto-rete\n10.0.1.0/24\n254 hosts}}
s17{{sotto-rete\n10.0.0.0/25\n126 hosts}}
s18{{sotto-rete\n10.0.0.128/25\n126 hosts}}


style s12 stroke:#f00,stroke-width:4px
style s14 stroke:#0f0,stroke-width:4px
style s17 stroke:#00f,stroke-width:4px

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
    s11 -- 1 --> s13
    s11 -- 1 --> s14
    s13 -- 1 --> s15
    s13 -- 1 --> s16
    s15 -- 1 --> s17
    s15 -- 1 --> s18
```

---

```mermaid
flowchart TB


r{{router}}

subgraph lA[LanA 10.0.4.0/22]
    s[server\n10.0.4.1]
end

subgraph lB[LanB 10.0.2.0/23]
    c1[client1\n10.0.2.1]
end

subgraph lC[LanC 10.0.0.0/25]
    c2[client2\n10.0.0.1]
end

style lA stroke:#f00
style lB stroke:#0f0
style lC stroke:#00f

    r --10.0.4.254--- lA
    r --10.0.2.254--- lB
    r --10.0.0.127--- lC
```

### Soluzione: Realizzazione del networking

#### Server

```shell
# Server
ip addr add 10.0.4.1/22
ip route add default via 10.0.4.254
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 10.0.4.1/22
    gateway 10.0.4.254
```

#### Client 1

```shell
# Client1
ip addr add 10.0.2.1/23
ip route add default via 10.0.2.254
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 10.0.2.1/23
    gateway 10.0.2.254

```

#### Client2

```shell
# Client2
ip addr add 10.0.0.1/25
ip route add default via 10.0.0.127
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 10.0.0.1/25
    gateway 10.0.0.127
```

#### Router

```shell
# Router
ip link set enp0s8 up
ip link set enp0s9 up
ip addr add 10.0.4.254/22 dev enp0s3
ip addr add 10.0.2.254/23 dev enp0s8
ip addr add 10.0.0.127/25 dev enp0s9
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 10.0.4.254/22

auto enp0s8
iface enp0s8 inet static
    address 10.0.2.254/23

auto enp0s9
iface enp0s9 inet static
    address 10.0.0.127/25
```

```py
# Router
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

### Soluzione: Programmazione Socket

[server.c](./server.c)  
[client.c](./client.c)
