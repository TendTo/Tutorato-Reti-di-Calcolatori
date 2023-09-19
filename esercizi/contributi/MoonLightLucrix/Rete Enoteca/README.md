# Rete Enoteca!
L'azienda **_Rete Enoteca_** ha deciso di sviluppare una piattaforma per la verifica di vini da differenti aziende.

I _Client-Azienda_ dovranno inviare i prodotto disponibili dalla propria azienda al _Server-Enoteca_.
In particolare utilizzando l'apposita **_struttura articolo_** composta dai seguenti campi _(Nome_Azienda, id_articolo, Nome_Vino, Quantità, Costo)_, per ogni articolo dovrà essere inviato un pacchetto contenente le seguenti informazioni:
- _Nome_Azienda_, _Nome_Vino_, _Quantità_, _Costo_
    - <ins>**NOTA</ins>: il campo _id_articolo_ (non presente nella lista di cui sopra) dovrà essere aggiunto dal _Server-Enoteca_ secondo una qualsiasi euristica definita dallo studente<ins>!</ins>**
- Ogni azienda, dopo aver richiesto al Server-Enoteca la propria listi dei vini disponibili potrà aggiornare la quantita di un singolo articolo specificando l'_'id_articolo_ e la _Quantità_.

I _Client-Clienti_ avranno la possibilità di chiedere al server la lista dei prodotti presenti nel _Server-Enoteca_. Quando il Client deciderà di acquistare un articolo specifico, dovrà indircare l'_id_articolo_ e la _Quantità_. In questo caso il _Server-Enoteca_ dovrà verificare la quantità disponibile e, in caso positivo decrementare la quantità dello specifico articolo. Nel caso in cui la quantità non è disponibile, il _Server-Enoteca_ invierà al client la quantità disponibile.


**Nota 1: Lo studente può definire qualsiasi approccio al fini di distringuere i _Client-Clienti_ dai _Client-Azienda_!**

**Nota 2: È consigliato l'uso di un File di testo come "memoria condivisa" per aggiornare e gestire tutte le operazioni sugli articoli di tutte le aziende. Ad ogni operazione di aggiunta e/o aggiornamento di quantità di un prodotto, verrà aggiornata la _struttura articolo_ e di conseguenza il file di testo. Quindi, ogni volta che il client fa una richiesta, il _Server-Enoteca_ aggiornerà la propria struttura con le informazioni disponibili nel file!**

Progettare ed Implementare la **_Rete Enoteca!_** tramite <ins>**_Socket TCP_**</ins> e <ins>**_Protocollo IPv6_**</ins>. 