# Il pacchetto Christmas

## Definizione dello scenario

### Progettazione della topologia

Configurare la rete secondo lo schema riportato in figura. Gli indirizzi IPv4 da utilizzare sono nel range $165.64.0.0/16$.

- Lan1 sia in grado di ospitare fino a 2000 Hosts.
  - I due **client** appartengono a questa rete
- Lan2 sia in grado di ospitare fino a 500 Hosts.
  - Il **server** appartiene a questa rete

```mermaid
flowchart TB

r{{router}}

subgraph l1[Lan1 - 2000 Hosts]
    c1[Client 1]
    c2[Client 2]
end

subgraph l2[Lan2 - 500 Hosts]
    s[Server]
end

    r --- l1 & l2
```

[Soluzione](#soluzione-progettazione-della-topologia)

### Realizzazione del networking

Dopo aver abbozzato la topologia su carta, si realizzi il networking definito in precedenza con delle macchine virtuali.  
Le macchine devono essere in grado di pingare tutte le altre.
È necessario che siano presenti

- 2 macchine nella Lan 1
- 1 macchina nella Lan 2
- 1 router che metta in comunicazione le due Lan

[Soluzione](#soluzione-realizzazione-del-networking)

### Programmazione Socket

Gli studenti di informatica del corso di Reti di Calcolatori hanno deciso di implementare un server Christmas in grado di ricevere e gestire bigliettini di Natale contenenti messaggi del tipo "Oggi supererò il laboratorio di Reti".  
Un generico client avrà la possibilità di:

- inviare un messaggio al server
- fare una richiesta per ricevere tutti i messaggi già inviati (solo dello specifico client).
- modificare e/o cancellare un messaggio specifico.

Il server dovrà essere progettato in modo da memorizzare qualsiasi messaggio (associandolo allo specifico client) e gestire le operazioni precedentemente elencate.
Quando il client invierà il carattere `q`, il programma (per lo specifico client) terminerà.

Si utilizzino le Socket **UDP** per la comunicazione.

> **Nota**  
> Lo studente può definire qualsiasi approccio per risolvere l'esercizio.

[Soluzione](#soluzione-programmazione-socket)

---

## Soluzioni

> **Note**  
> Prima di leggere le soluzioni, provare a risolvere l'esercizio da soli.
> Dopo averlo fatto, confrontare la propria soluzione con quella proposta.
> Ci sono tantissimi modi per risolvere le varie consegne, quindi non c'è da preoccuparsi se la propria soluzione è diversa da quella proposta.

### Soluzione: Progettazione della topologia

```mermaid
graph TD
r{{rete\n165.64.0.0/16\n64k hosts}}
s1{{sotto-rete\n165.64.0.0/17\n32k hosts}}
s2{{sotto-rete\n165.64.128.0/17\n32k hosts}}
s3{{sotto-rete\n165.64.0.0/18\n16k hosts}}
s4{{sotto-rete\n165.64.64.0/18\n16k hosts}}
s5{{sotto-rete\n165.64.0.0/19\n8k hosts}}
s6{{sotto-rete\n165.64.32.0/19\n8k hosts}}
s7{{sotto-rete\n165.64.0.0/20\n4k hosts}}
s8{{sotto-rete\n165.64.16.0/20\n4k hosts}}
s9{{sotto-rete\n165.64.0.0/21\n2046 hosts}}
s10{{sotto-rete\n165.64.8.0/21\n2046 hosts}}
s11{{sotto-rete\n165.64.8.0/22\n1022 hosts}}
s12{{sotto-rete\n165.64.12.0/22\n1022 hosts}}
s13{{sotto-rete\n165.64.8.0/23\n510 hosts}}
s14{{sotto-rete\n165.64.10.0/23\n510 hosts}}

style s9 stroke:#f00,stroke-width:4px
style s13 stroke:#0f0,stroke-width:4px

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
    s10 -- 0 --> s11
    s10 -- 1 --> s12
    s11 -- 0 --> s13
    s11 -- 1 --> s14
```

---

```mermaid
flowchart TB


r{{router}}

subgraph l1[Lan1 165.64.0.0/21]
    c1[Client 1\n165.64.0.1]
    c2[Client 2\n165.64.0.2]
end

subgraph l2[Lan2 165.64.8.0/23]
    s[Server\n165.64.8.1]
end

style l1 stroke:#f00
style l2 stroke:#0f0

    r --165.64.7.254--- l1
    r --165.64.9.254--- l2
```

### Soluzione: Realizzazione del networking

#### Client1

```shell
# Client1
ip addr add 165.64.0.1/21
ip route add default via 165.64.7.254
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.64.0.1/21
    gateway 165.64.7.254

```

#### Client2

```shell
# Client2
ip addr add 165.64.0.2/21
ip route add default via 165.64.7.254
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.64.0.2/21
    gateway 165.64.7.254
```

#### Server

```shell
# Server
ip addr add 165.64.8.1/23
ip route add default via 165.64.9.254
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.64.8.1/23
    gateway 165.64.9.254
```

#### Router1

```shell
# Router1
ip link set enp0s8 up
# Indirizzi ip del router
ip addr add 165.64.7.254/21 dev enp0s3
ip addr add 165.64.9.254/23 dev enp0s8
# Forwarding
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.64.7.254/21

auto enp0s8
iface enp0s8 inet static
    address 165.64.9.254/23
```

```py
# Router1
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

### Soluzione: Programmazione Socket

[server.c](./server.c)  
[client.c](./client.c)
[definitions.h](./definitions.h)
