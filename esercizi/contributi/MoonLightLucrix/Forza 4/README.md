### Forza 4

Gli studenti del corso di Reti di Calcolatori del DMI - UNICT hanno deciso, di loro spontanea volontà, di realizzare il gioco di forza 4 in modalità client-server.

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