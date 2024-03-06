# Impiccato

## Definizione dello scenario

Gli studenti del corso di Reti di Calcolatori del DMI - UNICT hanno deciso, di loro spontanea volontà, di realizzare il gioco di forza 4 in modalità client-server.

### Progettazione della topologia

Configurare la rete secondo lo schema riportato in figura. Gli indirizzi ip da utilizzare sono nel range $134.16.0.0/16$

- Lan1 sia in grado di ospitare fino a 2000 Hosts.
  - Il **Client1** apparterrà a questa sotto-rete
- Lan2 sia in grado di ospitare fino a 1024 Hosts.
  - Il **Client2** apparterrà a questa sotto-rete
- Lan3 sia in grado di ospitare fino a 520 Hosts.
  - Il **Server** apparterrà a questa sotto-rete

Si ha completa libertà nell'assegnamento degli ip alle macchine in ogni sotto-rete, purché tale ip sia valido.

```mermaid
flowchart TB


r{{router}}

subgraph l1[Lan1 - 2000 Hosts]
    c1[Client 1]
end

subgraph l2[Lan2 - 1024 Hosts]
    c2[Client 2]
end

subgraph l3[Lan3 - 520 Hosts]
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

Due macchine client si sfideranno tra di loro a fine di vincere una partita di **Forza 4**, con una griglia 6x7.
Il server dovrà gestire la partita, controllare l'eventuale vincitore e notificare i client sullo stato della partita.
Ad ogni turno:

- Il server prima invierà al client di turno lo stato del gioco, ovvero la griglia di gioco con le pedine già posizionate.
- Ogni client ad ogni turno avrà la possibilità solo di indicare la colonna in cui posizionare la pedina (1 - 7). Se la mossa non è valida, il server dovrà notificare il client e chiedere di ripetere la mossa.
- Non appena il giocatore avrà posizionato la pedina, il server dovrà controllare se il giocatore ha vinto o meno. In caso di vittoria, il server dovrà notificare il client e terminare la partita.
- Se non è più possibile fare mosse, la partita finisce in pareggio.

Il server dovrà memorizzare in una struct client il nome del giocatore e tutte le informazioni necessarie a comunicare con il client successivamente.
Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.
I client seguono il protocollo onestamente.
Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).

Progettare ed implementare **Forza 4** tramite TCP o UDP, considerando l'architettura vista prima.

#### Note e suggerimenti

- Si vince quando si allineano 4 pedine dello stesso colore in orizzontale, verticale o diagonale.
- I client dovranno attendere il proprio turno per giocare, probabilmente bloccati da una `recvfrom()`.
- Ad ogni mossa, il client indica solo la colonna dove inserire la pedina, che "cadrà" riempendo la prima casella vuota della colonna.
- Non è possibile inserire pedine in una colonna già piena.
- Ogni turno verrà inviata la griglia, che avrà l'aspetto di qualcosa come:
  ```text
  | | | | | | | |
  | | | | | | |x|
  | | | | | | |o|
  |x| |x| | | |o|
  |o| |o|x| |o|x|
  |o|x|o|x|x|x|o|
  ```

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

> [!Note]  
> Prima di leggere le soluzioni, provare a risolvere l'esercizio da soli.
> Dopo averlo fatto, confrontare la propria soluzione con quella proposta.
> Ci sono tantissimi modi per risolvere le varie consegne, quindi non c'è da preoccuparsi se la propria soluzione è diversa da quella proposta.

### Soluzione: Progettazione della topologia

```mermaid
graph TD
r{{rete\n134.16.0.0/16\n64k hosts}}
s1{{sotto-rete\n134.16.0.0/17\n32k hosts}}
s2{{sotto-rete\n134.16.128.0/17\n32k hosts}}
s3{{sotto-rete\n134.16.0.0/18\n16k hosts}}
s4{{sotto-rete\n134.16.64.0/18\n16k hosts}}
s5{{sotto-rete\n134.16.0.0/19\n8k hosts}}
s6{{sotto-rete\n134.16.32.0/19\n8k hosts}}
s7{{sotto-rete\n134.16.0.0/20\n4k hosts}}
s8{{sotto-rete\n134.16.16.0/20\n4k hosts}}
s9{{sotto-rete\n134.16.0.0/21\n2k hosts}}
s10{{sotto-rete\n134.16.8.0/21\n2k hosts}}
s11{{sotto-rete\n134.16.16.0/21\n2k hosts}}
s12{{sotto-rete\n134.16.24.0/21\n2k hosts}}
s13{{sotto-rete\n134.16.16.0/22\n1022 hosts}}
s14{{sotto-rete\n134.16.20.0/22\n1022 hosts}}

style s9 stroke:#f00,stroke-width:4px
style s10 stroke:#0f0,stroke-width:4px
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
    s8 -- 0 --> s11
    s8 -- 0 --> s12
    s11 -- 0 --> s13
    s11 -- 1 --> s14
```

---

```mermaid
flowchart TB


r{{router}}

subgraph l1[Lan1 134.16.0.0/21]
    c1[Client 1\n134.16.0.1]
end

subgraph l2[Lan2 134.16.8.0/21]
    c2[Client 2\n134.16.8.1]
end

subgraph l3[Lan3 134.16.16.0/22]
    s[Server\n134.16.16.1]
end

style l1 stroke:#f00
style l2 stroke:#0f0
style l3 stroke:#00f

    r --134.16.0.254--- l1
    r --134.16.8.254--- l2
    r --134.16.16.254--- l3
```

### Soluzione: Realizzazione del networking

#### Client 1

```shell
# Client1
ip addr add 134.16.0.1/21
ip route add default via 134.16.0.254
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 134.16.0.1/21
    gateway 134.16.0.254

```

#### Client2

```shell
# Client2
ip addr add 134.16.8.1/21
ip route add default via 134.16.8.254
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 134.16.8.1/21
    gateway 134.16.8.254
```

#### Server

```shell
# Server
ip addr add 134.16.16.1/22
ip route add default via 134.16.16.254
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 134.16.16.1/22
    gateway 134.16.16.254
```

#### Router

```shell
# Router
ip link set enp0s8 up
ip link set enp0s9 up
ip addr add 134.16.0.254/21 dev enp0s3
ip addr add 134.16.8.254/21 dev enp0s8
ip addr add 134.16.16.254/22 dev enp0s9
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 134.16.0.254/21

auto enp0s8
iface enp0s8 inet static
    address 134.16.8.254/21

auto enp0s9
iface enp0s9 inet static
    address 134.16.16.254/22
```

```py
# Router
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

### Soluzione: Programmazione Socket

[server.c](./server.c)  
[client.c](./client.c)
