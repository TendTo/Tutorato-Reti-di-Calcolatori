# Server-Farm
Una farmacia ha chiesto agli studenti di informatica del corso di Reti di Calcolatori di implementare un server **_Server-Farm_** al fine di agevolare e accelerare le vendite dei prodotti farmaceutici.

Il **_Server-Farm_** memorizza in un **file binario** le seguenti informazioni per ogni articolo:
- _id, nome_farmaco, quantità_

Un generico cliente potrà decidere se acquistare un prodotto in presenza (_Cliente<sub>presente</sub>_) o tramite app (_Cliente<sub>remoto</sub>_). Dopo aver stabilito una connessione con il server, il **generico client riceverà una lista dei farmaci disponibili**.<br>
Il client invierà al **_Server-Farm_** _l'id_ del farmaco da acquistare. Il **_Server-Farm_** verificherà inizialmente la presenza del farmaco (se l'_id_ è presente). In caso positivo, il server controllerà la quantità disponibile e restituirà il messaggio <ins>_"Farmaco comprato!"_</ins> nel caso di quantità disponibile, altrimenti verrà restituito il messaggio <ins>_"Errore nell'acquisto del farmaco! Quantità non disponibile."_</ins>.<br>
Se il cliente inserisce un _id_ non disponibile nella lista, allora il server restituirà un messaggio di errore.<br>
Nel caso di _Cliente<sub>remoto</sub>_ dovrà essere inviato anche l'indirizzo di spedizione del farmaco. Il **_Server-Farm_** memorizzerà in un **file di testo** **_spedizioni.txt_** l'indirizzo, il nome del farmaco acquistato e la quantità.<br>

Quando il client invierà il carattere "q", avrà concluso il suo acquisto.

**Nota: lo studente può definire qualsiasi approccio al fine di distinguere i** _Cliente<sub>presente</sub>_ **dai** _Cliente<sub>remoto</sub>_**!**

Progettare e implementare **_Server-Farm!_** tramite **<ins>Socket TCP</ins>** e **<ins>Protocollo IPv6</ins>**.