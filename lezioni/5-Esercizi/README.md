# Esercizi

Esercizi di teoria.

<!-- New section -->

## Formulario

Lista di formule generali per risolvere gli esercizi.

<!-- New subsection -->

### Legenda

$$
\begin{array}{ll}
BW & \text{Banda (bps)}
\newline
BW_{s \to d} & \text{Banda dalla sorgente alla destinazione (bps)}
\newline
BW_{d \to s} & \text{Banda dalla destinazione alla sorgente (bps)}
\newline
BW_e & \text{Banda effettiva (bps)}
\end{array}
$$

<!-- New subsection -->

### Legenda

$$
\begin{array}{ll}
T_p & \text{Ritardo di propagazione (s)}
\newline
T_{tf} & \text{Ritardo di trasmissione frame (s)}
\newline
T_{ta} & \text{Ritardo di trasmissione ACK (s)}
\newline
T_{e} & \text{Ritardo dovuto ad errori (s)}
\newline
RTT & \text{Round Trip Time (s)}
\newline
T_{t} & \text{Tempo totale}
\end{array}
$$

<!-- New subsection -->

### Legenda

$$
\begin{array}{ll}
I & \text{Intestazione (bit)}
\newline
P & \text{Payload (bit)}
\newline
L & \text{Perdita (percentuale)}
\newline
t & \text{Timer (s)}
\newline
W & \text{Finestra (pacchetti)}
\newline
N_f & \text{Numero di frame effettivo in una finestra (pacchetti)}
\end{array}
$$

<!-- New subsection -->

### Tempi senza errori

$$
T_p = \frac{\text{Lunghezza da percorrere}}{\text{Velocità del segnale}}
\newline \space
\newline
T_{tf} = \frac{I + P}{BW_{s \to d}}
\qquad
T_{ta} = \frac{I}{BW_{d \to s}}
\newline \space
\newline
RTT = 2 \cdot T_p
\newline \space
\newline
T_{t} = RTT + T_{tf} + T_{ta}
$$

<!-- New subsection -->

### Tempi in presenza di errori

$$
\begin{array}{ll}
T_e = 2 \cdot L \cdot t
\newline \space
\newline
T_{t} = RTT + T_{tf} + T_{ta} + T_e
\end{array}
$$

<!-- New subsection -->

### Banda effettiva

$$
BW_e = \frac{P}{T_t}
\newline \space
\newline
BW_e = \frac{P}{RTT + T_{tf} + T_{ta} + T_e}
\newline \space
\newline
BW_e = \frac{P}{2 \cdot T_p + \frac{I + P}{BW_{s \to d}} + \frac{I}{BW_{d \to s}} + 2 \cdot L \cdot t}
$$

<!-- New subsection -->

### Banda effettiva con finestra (Go-Back-N)

$$
\begin{array}{ll}
N_f = \frac{RTT}{T_{tf}}
\end{array}
\newline \space
\newline
\begin{cases}
BW_e = \frac{W \cdot P}{T_t} & \text{se } W \leq N_f
\newline \space
\newline
BW_e = \frac{\lfloor N_f \rfloor \cdot P}{T_t} & \text{se } W > N_f
\end{cases}
$$

<!-- New section -->

## Protocollo stop-and-wait

Un protocollo stop-and-wait è un protocollo di comunicazione che prevede che il mittente invii un pacchetto e attenda la ricezione di un ACK prima di inviare il pacchetto successivo.

<!-- New subsection -->

### Schema

```mermaid
sequenceDiagram
participant m as Mittente
participant d as Destinatario

m ->> d : frame 1
d -->> m : ACK 1
m ->> d : frame 2
d -->> m : ACK 2
```

<!-- New subsection -->

### Esercizio 1

$$
\begin{array}{lll}
\text{Banda}(BW) &= 8 \text{ Mbps} &= 8 \cdot 10^6 \text{ bps}
\newline
\text{Ritardo di propagazione}(T_p) &= 10 \text{ ms} &= 10^{-2} \text{ s}
\newline
\text{Intestazione}(I) &= 10 \text{ byte} &= 80 \text{ bit}
\newline
\text{Banda effettiva}(BW_e) &= 5 \text{ Mbps} &= 5 \cdot 10^6 \text{ bps}
\newline
\text{Timer}(t) &= 100 \text{ ms} &= 10^{-1} \text{ s}
\newline
\text{Perdita}(L) &= 0.01
\end{array}
$$

$$
\begin{array}{ll}
\text{Ritardo di trasmissione frame}(T_{tf}) &= \text{ ?}
\newline
\text{Ritardo di trasmissione ACK}(T_{ta}) &= \text{ ?}
\newline
\text{Dimensione Payload}(P) &= \text{ ?}
\end{array}
$$

<!-- New subsection -->

### Formule

$$
\begin{array}{lll}
& BW_e &=
\newline
\newline
=& \frac{P}{RTT + T_{tf} + T_{ta} + L \cdot 2 \cdot t} &=
\newline
\newline
=& \frac{P}{2 \cdot T_p + \frac{I + P}{BW} + \frac{I}{BW} + L \cdot 2 \cdot t}
\end{array}
$$

<!-- New subsection -->

### Ricavare P

$$
\begin{array}{ll}
BW_e(2 \cdot T_p + \frac{I + P}{BW} + \frac{I}{BW} + L \cdot 2 \cdot t) &= P
\newline
BW_e \cdot BW(2 \cdot T_p + \frac{I + P}{BW} + \frac{I}{BW} + L \cdot 2 \cdot t) &= BW \cdot P
\newline
BW_e ( BW \cdot 2 \cdot T_p + I + P + I + BW \cdot L \cdot 2 \cdot t) &= BW \cdot P
\newline
BW_e \cdot P + 2 \cdot BW_e ( BW \cdot T_p + I + BW \cdot L \cdot t) &= BW \cdot P
\newline
2 \cdot BW_e ( BW \cdot T_p + I + BW \cdot L \cdot t) &= BW \cdot P - BW_e \cdot P
\newline
2 \cdot BW_e ( BW \cdot T_p + I + BW \cdot L \cdot t) &= (BW - BW_e) \cdot P
\newline
\frac{2 \cdot BW_e ( BW \cdot T_p + I + BW \cdot L \cdot t)}{BW - BW_e} &= P
\newline
\frac{10 \cdot 10^6 ( 8 \cdot 10^4 + 80 + 8 \cdot 10^3)}{3 \cdot 10^6} &= P
\end{array}
$$

$$
P = 293600 \text{ bit}
\newline
P = 36700 \text{ byte}
$$

<!-- New section -->

## Protocollo go back n

Un protocollo go back n è un protocollo di comunicazione che prevede che il mittente invii un pacchetto certo numero di pacchetti e attenda la ricezione di un ACK prima di inviarne un altro. In caso di perdita di un pacchetto, il mittente invia nuovamente tutti i pacchetti successivi a quello perso.

<!-- New subsection -->

### Schema

```mermaid
sequenceDiagram
participant m as Mittente
participant d as Destinatario

m ->> d : frame 1
m ->> d : frame 2
m -x d : frame 3

d -->> m : ACK 1
m -x d : frame 4
d -->> m : ACK 2
m -x d : frame 5

note over m: Scade il timer di frame 3
m ->> d : frame 3
d -->> m : ACK 3
```

<!-- New subsection -->

### Esercizio 1

$$
\begin{array}{lll}
\text{Ritardo di propagazione}(T_p) &= 10 \text{ ms} &= 10^{-2} \text{ s}
\newline
\text{Banda}(BW) &= 100 \text{ Mbps} &= 10^8 \text{ bps}
\newline
\text{Intestazione}(I) &= 100 \text{ byte} &= 800 \text{ bit}
\newline
\text{Dimensione payload}(P) &= 1000 \text{ byte} &= 8000 \text{ bit}
\newline
\text{Window}(W) &= 20
\end{array}
$$

$$
\text{Lunghezza di banda effettiva}(BW_e) = \text{ ?}
$$

<!-- New subsection -->

### Formule

$$
\begin{array}{lll}
\text{Trasferimento frame}(T_{tf}) &= \frac{I + P}{BW} &= \frac{800 + 8000}{10^8} &= 8.8 \cdot 10^{-5} \text{ s}
\newline
\newline
\text{Numero di frame in } RTT &= \frac{2 \cdot T_p}{T_f} &= \frac{2 \cdot 10^{-2}}{8.8 \cdot 10^{-5}} &\approx 227.27
\end{array}
$$

<!-- New subsection -->

### Ricavare BWe

Poiché il numero di frame che sarebbe possibile inviare nell'RTT è maggiore o uguale alla finestra massima stabilita, prendiamo in considerazione quella: 20.

$$
\begin{array}{lll}
BW_e &= \frac{P \cdot W}{2 \cdot T_p  + T_{tf} + T_{ta}}
\newline
\newline
BW_e &= \frac{8000 \cdot 20}{2 \cdot 10^{-2} + 8.8 \cdot 10^{-5}}
\newline
\newline
BW_e &\approx 8 \cdot 10^6 \text{ bps} &= 8 \text{ Mbps}
\end{array}
$$

<!-- New subsection -->

### Esercizio 2

$$
\begin{array}{lll}
\text{Window}(W) &= 5
\newline
\text{Band Width}(BW) &= 12 \text{ Mbit/s} &= 12 \cdot 10^6 \text{ bit/s}
\newline
\text{Intestazione}(I) &= 100 \text{ byte} &= 800 \text{ bit}
\newline
\text{PayLoad}(P) &= 900 \text{ byte} &= 7200 \text{ bit}
\newline
\text{Lunghezza canale}(L) &= 100 \text{ km} &= 10^5 \text{ m}
\newline
\text{Velocità del segnale}(V) &= 200000 \text{ km/s} &= 2 \cdot 10^8 \text{ m/s}
\end{array}
$$

$$
\text{Lunghezza di banda effettiva}(BW_e) = \text{ ?}
$$

<!-- New subsection -->

### Formule

$$
\begin{array}{llll}
T_p &= \frac{L}{V} &= \frac{10^5}{2 \cdot 10^8} &= 5 \cdot 10^{-4} \text{ s}
\newline
\newline
T_{tf} &= \frac{I + P}{BW} &= \frac{800 + 7200}{12 \cdot 10^6} &\approx 6 \cdot 10^{-4} \text{ s}
\newline
\newline
T_{ta} &= \frac{I}{BW} &= \frac{800}{12 \cdot 10^6} &\approx 6 \cdot 10^{-5} \text{ s}
\newline
\newline
T_t &= 2 \cdot T_p + T_{tg} + T_{ta} &= 10^{-3} + 6.6 \cdot 10^{-4}  &= 1.66 \cdot 10^{-3} \text{ s}
\newline
\newline
N_f &= \left\lfloor \frac{2 \cdot T_p}{T_{tf}} \right\rfloor &= \left\lfloor \frac{10^{-3}}{6 \cdot 10^{-4}} \right\rfloor &\approx \lfloor 1.6 \rfloor = 1
\end{array}
$$

<!-- New subsection -->

### Ricavare BWe

Poiché il numero di frame che è possibile inviare nell'RTT è minore della finestra massima stabilita, prendiamo in considerazione il più piccolo: 1.

$$
\begin{array}{lll}
BW_e &= \frac{P \cdot 1}{T_t}
\newline
\newline
BW_e &= \frac{7200 \cdot 1}{1.66 \cdot 10^{-3}}
\newline
\newline
BW_e &\approx 4.33 \cdot 10^6 \text{ bps} &= 4.33 \text{ Mbps}
\end{array}
$$

<!-- New subsection -->

### Esercizio 3

$$
\begin{array}{lll}
\text{Ritardo di propagazione}(T_p) &= 100 \text{ ms} &= 10^{-1} \text{ s}
\newline
\text{Band Width andata}(BW_{s \to d}) &= 1 \text{ Mbit/s} &= 10^6 \text{ bit/s}
\newline
\text{Band Width ritorno}(BW_{d \to s}) &= 10 \text{ Kbit/s} &= 10^4 \text{ bit/s}
\newline
\text{Intestazione}(I) &= 1000 \text{ byte} &= 8000 \text{ bit}
\newline
\text{PayLoad}(P) &= 9000 \text{ byte} &= 72000 \text{ bit}
\newline
\text{ACK}(A) &= 100 \text{ byte} &= 800 \text{ bit}
\end{array}
$$

Con la condizione che la banda minima a livello superiore sia maggiore di 500 Kbps (500000 bps), calcolare la finestra di trasmissione.

$$
\text{Window}(W) = \text{ ?}
$$

<!-- New subsection -->

### Formule

$$
\begin{array}{llll}
T_{tf} &= \frac{I + P}{BW_{s \to d}} &= \frac{8000 + 72000}{10^6} &= 0.08 \text{ s}
\newline
\newline
T_{ta} &= \frac{I + A}{BW_{d \to s}} &= \frac{8000 + 800}{10^4} &= 0.88 \text{ s}
\newline
\newline
T_t &= 2 \cdot T_p + T_{tf} + T_{ta} &= 0.2 + 0.08 + 0.88 &= 1.16 \text{ s}
\end{array}
$$

<!-- New subsection -->

### Ricavare W

$$
\begin{array}{lll}
BW_e &= \frac{W \cdot P}{T_t} &\ge 500000
\newline
\newline
W &\ge \left\lceil \frac{500000 \cdot T_t}{P} \right\rceil
\newline
\newline
W &\ge \left\lceil \frac{500000 \cdot 1.16}{72000} \right\rceil &= 9
\end{array}
$$

<!-- New section -->

## Cyclic Redundancy Check (CRC)

Il CRC è un codice di controllo a ridondanza ciclica che permette di rilevare errori di trasmissione.  
Il CRC viene calcolato dal mittente e viene inviato insieme al messaggio.
Il destinatario calcola il CRC del messaggio ricevuto e lo confronta con quello ricevuto.  
Se i due CRC coincidono, il messaggio è stato trasmesso correttamente.

<!-- New subsection -->

### Esercizio 1

$M = 101101 \quad G = 1101 \quad |G| - 1 = 4 - 1 = 3$

Calcolare il CRC.

$$
\begin{array}{ll}
101101 \space 000
\newline
1101
\newline
011001 \space 000
\newline
\space\space 1101
\newline
000011 \space 000
\newline
\space\space \space\space \space\space \space\space 11 \space 01
\newline
000000 \space \underbrace{010}_{\text{resto}}
\end{array}
$$

<!-- New subsection -->

### Esercizio 2

$M = 1101101 \qquad G = 11010 \qquad |G| - 1 = 4$

Calcolare il CRC.

$$
\begin{array}{ll}
1101101 \space 0000
\newline
11010
\newline
0000101 \space 0000
\newline
\space\space \space\space \space\space \space\space 110 \space 10
\newline
0000011 \space 1000
\newline
\space\space \space\space \space\space \space\space \space\space 11 \space 010
\newline
0000000 \space \underbrace{1100}_{\text{resto}}
\end{array}
$$

<!-- New subsection -->

### Esercizio 3

$M = 101001 \space 011 \qquad G = 1001 \qquad |G| - 1 = 3$

Verificare se il messaggio è stato trasmesso correttamente.

$$
\begin{array}{ll}
101001 \space 011
\newline
1001
\newline
001101 \space 011
\newline
\space\space \space\space 1001
\newline
000100 \space 011
\newline
\space\space \space\space \space\space 100 \space 1
\newline
000000 \space 111 & \ne 0 \to \text{errore}
\end{array}
$$

<!-- New subsection -->

### Esercizio 4

$M = 101101 \space 0000 \qquad G = 11011 \qquad |G| - 1 = 4$

Verificare se il messaggio è stato trasmesso correttamente.

$$
\begin{array}{ll}
101101 \space 0000
\newline
11011
\newline
011011 \space 0000
\newline
\space\space 11011
\newline
000000 \space 0000 & = 0 \to \text{corretto}
\end{array}
$$

<!-- New subsection -->

### Esercizio 5

$M = 1001 \space 110 \qquad G = 1011 \qquad |G| - 1 = 3$

Verificare se il messaggio è stato trasmesso correttamente.

$$
\begin{array}{ll}
1001 \space 110
\newline
1011
\newline
0010 \space 110
\newline
\space\space \space\space 10 \space 11
\newline
0000 \space 000 & = 0 \to \text{corretto}
\end{array}
$$

<!-- New section -->

## Distanza di Hamming

La distanza di Hamming è la distanza tra due parole di uguale lunghezza, definita come il numero di posizioni in cui le due parole differiscono.

<!-- New subsection -->

### Esercizio 1

Fanno parte del dizionario tutte le parole di 3 bit dove i primi 2 vengono usati per trasferire i dati e l'ultimo viene usato per il controllo di parità.

$$
\begin{array}{ll}
\text{Parole valide} & \text{Parole non valide}
\newline
000 & 001
\newline
011 & 010
\newline
101 & 100
\newline
110 & 111
\end{array}
$$

<!-- New subsection -->

#### Distanza minima

La distanza minima è la distanza più piccola tra tutte le coppie di parole diverse del dizionario.

$$
\begin{bmatrix}
& 000 & 011 & 101 & 110
\newline
000 & 0 & 2 & 2 & 2
\newline
011 & 2 & 0 & 2 & 2
\newline
101 & 2 & 2 & 0 & 2
\newline
110 & 2 & 2 & 2 & 0
\end{bmatrix}
$$

La distanza più piccola fra parole diverse è 2, quindi la distanza minima è 2.
