# Caffè Sbagliato

## Definizione dello scenario

### Progettazione della topologia

Configurare la rete secondo lo schema riportato in figura.
Gli indirizzi IPv4 da utilizzare sono nel range $165.11.0.0/16$.
Si supponga che

- Lan1 sia in grado di ospitare fino a 5k Hosts.
  - **Client1** appartiene a questa rete
- Lan2 sia in grado di ospitare fino a 800 Hosts.
  - **Client2** appartiene a questa rete
- Lan3 sia in grado di ospitare fino a 15k Hosts.
  - **Client3** appartiene a questa rete
- Lan4 sia in grado di ospitare fino a 800 Hosts.
  - **Server** appartiene a questa rete

```mermaid
block-beta
  columns 7
  space r2{{"router 2"}}  space r3{{"router 3"}} space r4{{"router 4"}} space
  block:l1
  columns 1
  l11["Lan 1 - 5k hosts"] c1["Client 1"]
  end
  space
  block:l2
  columns 1
  l22["Lan 2 - 800  hosts"] c2["Client 2"]
  end
  space
  block:l3
  columns 1
  l33["Lan 3 - 15k  hosts"] c3["Client 3"]
  end
  space
  block:l4
  columns 1
  l44["Lan 4 - 800k  hosts"] s["Server"]
  end
  space:3 r1{{"router 1"}}   space:3

r1 --> l1
r1 --> l4
r2 --> l1
r2 --> l2
r3 --> l2
r3 --> l3
r4 --> l3
r4 --> l4

style l11 fill:transparent,stroke:transparent
style l22 fill:transparent,stroke:transparent
style l33 fill:transparent,stroke:transparent
style l44 fill:transparent,stroke:transparent
```

[Soluzione](#soluzione-progettazione-della-topologia)

### Realizzazione del networking

Dopo aver abbozzato la topologia su carta, si realizzi il networking definito in precedenza con delle macchine virtuali.  
Le macchine devono essere in grado di pingare il server.
È necessario che siano presenti

[Soluzione](#soluzione-realizzazione-del-networking)

### Programmazione Socket

Gli studenti del corso di Reti di Calcolatori hanno deciso di implementare un tool per l'impresa "Cocco Bello" al fine di ottimizzare le vendite nelle spiagge libere di Catania.
L'impresa metterà a disposizione una sequenza di prodotti con prezzo e quantità a disposizione.
Un generico cliente richiederà la lista dei prodotti disponibili e potrà avere la possibilità di acquistare uno o piu prodotti specificandone la quantità.
Dopo l'acquisto di un prodotto, la relative quantità verrà aggiornata.
Il servizio deve prevedere la possibilità autenticare un utente prima dell'acquisto di un prodotto (_non necessariamente deve essere implementata la funzione di registrazione_).

**Usare Socket TCP.**

> [!Note]  
> Lo studente può definire qualsiasi approccio per risolvere l'esercizio.

---

## Soluzioni

> [!Note]  
> Prima di leggere le soluzioni, provare a risolvere l'esercizio da soli.
> Dopo averlo fatto, confrontare la propria soluzione con quella proposta.
> Ci sono tantissimi modi per risolvere le varie consegne, quindi non c'è da preoccuparsi se la propria soluzione è diversa da quella proposta.

### Soluzione: Progettazione della topologia

Calcolo delle maschere necessarie per soddisfare i requisiti di ciascuna lan:

$$
\begin{gather*}
\text{Lan1}: \lceil \log_2(5000) \rceil = 13 \rightarrow 32 - 13 = 19 \newline
\text{Lan2}: \lceil \log_2(800) \rceil = 10 \rightarrow 32 - 10 = 22 \newline
\text{Lan3}: \lceil \log_2(15000) \rceil = 14 \rightarrow 32 - 14 = 18 \newline
\text{Lan3}: \lceil \log_2(800) \rceil = 10 \rightarrow 32 - 10 = 22
\end{gather*}
$$

```mermaid
graph TD
r{{"`rete<br>165.11.0.0/16<br>64k hosts`"}}
s1{{"`sotto-rete<br>165.11.0.0/17<br>32k hosts`"}}
s2{{"`sotto-rete<br>165.11.128.0/17<br>32k hosts`"}}
s3{{"`sotto-rete<br>165.11.0.0/18<br>16k hosts`"}}
s4{{"`sotto-rete<br>165.11.64.0/18<br>16k hosts`"}}
s5{{"`sotto-rete<br>165.11.0.0/19<br>8k hosts`"}}
s6{{"`sotto-rete<br>165.11.32.0/19<br>8k hosts`"}}
s7{{"`sotto-rete<br>165.11.0.0/20<br>4k hosts`"}}
s8{{"`sotto-rete<br>165.11.16.0/20<br>4k hosts`"}}
s9{{"`sotto-rete<br>165.11.0.0/21<br>2046 hosts`"}}
s10{{"`sotto-rete<br>165.11.8.0/21<br>2046 hosts`"}}
s11{{"`sotto-rete<br>165.11.0.0/22<br>1022 hosts`"}}
s12{{"`sotto-rete<br>165.11.4.0/22<br>1022 hosts`"}}

style s4 stroke:#00f,stroke-width:4px
style s6 stroke:#f00,stroke-width:4px
style s11 stroke:#0f0,stroke-width:4px
style s12 stroke:#f0f,stroke-width:4px

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
```

---

```mermaid
block-beta
  columns 7
  space r2{{"router 2"}}  space r3{{"router 3"}} space r4{{"router 4"}} space
  block:l1
  columns 1
  l11["Lan 1 - 165.11.32.0/19"] c1["Client 1\n165.11.32.1"]
  end
  space
  block:l2
  columns 1
  l22["Lan 2 - 165.11.0.0/22"] c2["Client 2\n165.11.0.1"]
  end
  space
  block:l3
  columns 1
  l33["Lan 3 - 165.11.64.0/18"] c3["Client 3\n165.11.64.1"]
  end
  space
  block:l4
  columns 1
  l44["Lan 4 - 165.11.4.0/22"] s["Server\n165.11.4.1"]
  end
  space:7
  space:3 r1{{"router 1"}}   space:3

r1 --"165.11.4.255"--> l4
r1 --"165.11.32.255"--> l1
r2 --"165.11.32.254"--> l1
r2 --"165.11.0.254"--> l2
r3 --"165.11.0.255"--> l2
r3 --"165.11.64.255"--> l3
r4 --"165.11.64.254"--> l3
r4 --"165.11.4.254"--> l4

style l11 fill:transparent,stroke:transparent
style l22 fill:transparent,stroke:transparent
style l33 fill:transparent,stroke:transparent
style l44 fill:transparent,stroke:transparent
style l1 stroke:#f00
style l2 stroke:#0f0
style l3 stroke:#00f
style l4 stroke:#f0f
```

### Soluzione: Realizzazione del networking

#### Client1

```shell
# Client1
ip addr add 165.11.32.1/19
ip route add default via 165.11.32.255
```

oppure

```py
# Client1
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.32.1/19
    gateway 165.11.32.255

```

#### Client2

```shell
# Client2
ip addr add 165.11.0.1/22
ip route add default via 165.11.0.255
```

oppure

```py
# Client2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.0.1/22
    gateway 165.11.0.255
```

#### Client 3

```shell
# Client3
ip addr add 165.11.64.1/18
ip route add default via 165.11.64.255
```

oppure

```py
# Client3
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.64.1/18
    gateway 165.11.64.255
```

#### Server

```shell
# Server
ip addr add 165.11.4.1/22
ip route add default via 165.11.4.255
```

oppure

```py
# Server
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.4.1/22
    gateway 165.11.4.255
```

#### Router1

```shell
# Router1
ip link set enp0s8 up
# Indirizzi ip del router
ip addr add 165.11.4.255/22 dev enp0s3
ip addr add 165.11.32.255/19 dev enp0s8
# Routing di default
ip route add default via 165.11.32.254
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
    address 165.11.4.255/22

auto enp0s8
iface enp0s8 inet static
    address 165.11.32.255/19
    gateway 165.11.32.254
```

```py
# Router1
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

#### Router2

```shell
# Router2
ip link set enp0s8 up
# Indirizzi ip del router
ip addr add 165.11.32.254/19 dev enp0s3
ip addr add 165.11.0.254/22 dev enp0s8
# Routing di default
ip route add default via 165.11.0.255
# Forwarding
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.32.254/19

auto enp0s8
iface enp0s8 inet static
    address 165.11.0.254/22
    gateway 165.11.0.255
```

```py
# Router1
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

#### Router3

```shell
# Router2
ip link set enp0s8 up
# Indirizzi ip del router
ip addr add 165.11.0.255/22 dev enp0s3
ip addr add 165.11.64.255/18 dev enp0s8
# Routing di default
ip route add default via 165.11.64.254
# Forwarding
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.0.255/22

auto enp0s8
iface enp0s8 inet static
    address 165.11.64.255/18
    gateway 165.11.64.254
```

```py
# Router1
# nano /etc/sysctl.conf
net.ipv4.ip_forward=1
```

#### Router4

```shell
# Router2
ip link set enp0s8 up
# Indirizzi ip del router
ip addr add 165.11.64.254/18 dev enp0s3
ip addr add 165.11.4.254/22 dev enp0s8
# Routing di default
ip route add default via 165.11.4.255
# Forwarding
sysctl -w net.ipv4.ip_forward=1
```

oppure

```py
# Router2
# nano /etc/network/interfaces
# ...
auto enp0s3
iface enp0s3 inet static
    address 165.11.64.254/18

auto enp0s8
iface enp0s8 inet static
    address 165.11.4.254/22
    gateway 165.11.4.255
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
