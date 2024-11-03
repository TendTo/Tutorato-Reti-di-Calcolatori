# Topologia di un Web Server a 2 livelli

## Descrizione

In questa topologia, ci si immagina un web server che riceve richieste dai client.
Poiché i client si trovano in una sottorete differente, tali messaggi verranno veicolati da un router dell' ISP (Internet Service Provider).  
Il server consulterà poi il database per ottenere le informazioni relative alla pagina web da servire al client.

Per motivi di sicurezza, il database si trova in una sottorete differente rispetto al server, e la comunicazione avviene tramite un router interno.
Il router dell'ISP non conosce il router interno, in maniera da rendere il database inaccessibile dall'esterno.  
Il server deve quindi essere in grado di decidere dove inviare i pacchetti, verso l'esterno o l'interno, in base all'indirizzo IP di destinazione.

## Topologia

```mermaid
flowchart LR

subgraph Internet_192.168.1.0/24
    c["`client<br>192.168.1.1`"]
end

subgraph Lan1_10.0.1.0/24
    s["`server<br>10.0.1.1`"]
end

subgraph Lan2_172.16.0.0/16
    db["`database<br>172.16.2.2`"]
end

re{{"`router ISP<br>192.168.1.254<br>10.0.1.254`"}}
rb{{"`router interno<br>10.0.1.253<br>172.16.1.253`"}}

re --192.168.1.254--- Internet_192.168.1.0/24
re --10.0.1.254--- Lan1_10.0.1.0/24
rb --10.0.1.253--- Lan1_10.0.1.0/24
rb --172.16.1.253--- Lan2_172.16.0.0/16
```
