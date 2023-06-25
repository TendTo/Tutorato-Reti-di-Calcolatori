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
\newline
\newline
T_p & \text{Ritardo di propagazione (s)}
\newline
T_{tf} & \text{Ritardo di trasmissione frame (s)}
\newline
T_{ta} & \text{Ritardo di trasmissione ACK (s)}
\newline
T_{e} & \text{Ritardo dovuto ad errori (s)}
\newline
RTT & \text{Round Trip Time (s)}
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
\newline \space
\newline
T_{ta} = \frac{I}{BW_{d \to s}}
\newline \space
\newline
RTT = 2 \cdot T_p + T_{tf} + T_{ta}
$$

<!-- New subsection -->

### Tempi in presenza di errori

$$
\begin{array}{ll}
T_e = 2 \cdot L \cdot t
\newline \space
\newline
RTT = 2 \cdot T_p + T_{tf} + T_{ta} + T_e
\end{array}
$$

<!-- New subsection -->

### Banda effettiva

$$
BW_e = \frac{P}{RTT}
\newline \space
\newline
BW_e = \frac{P}{2 \cdot T_p + T_{tf} + T_{ta} + T_e}
\newline \space
\newline
BW_e = \frac{P}{2 \cdot T_p + \frac{I + P}{BW_{s \to d}} + \frac{I}{BW_{d \to s}} + 2 \cdot L \cdot t}
$$

<!-- New subsection -->

### Banda effettiva con finestra (Go-Back-N)

$$
\begin{array}{ll}
N_f = \frac{2 \cdot T_p}{T_{tf}}
\end{array}
\newline \space
\newline
\begin{cases}
BW_e = \frac{W \cdot P}{RTT} & \text{se } W \leq N_f
\newline \space
\newline
BW_e = \frac{\lfloor N_f \rfloor \cdot P}{RTT} & \text{se } W > N_f
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
=& \frac{P}{2 \cdot T_p + T_{tf} + T_{ta} + L \cdot 2 \cdot t} &=
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
\text{Inserimento frame}(T_f) &= \frac{I + P}{BW} &= \frac{800 + 8000}{10^8} &= 8.8 \cdot 10^{-5} \text{ s}
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
BW_e &= \frac{P \cdot W}{T_p  + T_f}
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
T_{ta} &= \frac{P}{BW} &= \frac{7200}{12 \cdot 10^6} &= 6 \cdot 10^{-4} \text{ s}
\newline
\newline
RTT &= 2 \cdot T_p + T_{tg} + T_{ta} &= 10^{-3} + 12 \cdot 10^{-4} &= 2.2 \cdot 10^{-3} \text{ s}
\newline
\newline
N_f &= \left\lfloor \frac{2 \cdot T_p}{T_t} \right\rfloor &= \left\lfloor \frac{10^{-3}}{6 \cdot 10^{-4}} \right\rfloor &\approx \lfloor 1.6 \rfloor = 1
\end{array}
$$

<!-- New subsection -->

### Ricavare BWe

Poiché il numero di frame che è possibile inviare nell'RTT è minore della finestra massima stabilita, prendiamo in considerazione il più piccolo: 1.

$$
\begin{array}{lll}
BW_e &= \frac{P \cdot 1}{RTT}
\newline
\newline
BW_e &= \frac{7200 \cdot 1}{2.2 \cdot 10^{-3}}
\newline
\newline
BW_e &\approx 3.27 \cdot 10^6 \text{ bps} &= 3.27 \text{ Mbps}
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

Messaggio $M = 101101$, polinomio generatore $G = 1101$.  
Bit di ridondanza: $|G| - 1 = 4 - 1 = 3$.

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
