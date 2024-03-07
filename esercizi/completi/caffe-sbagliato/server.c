/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Gli studenti di informatica del corso di Reti di Calcolatori hanno deciso di implementare un sistema centralizzato per una nuova gestione delle macchinette del caffè del DMI posizionate nei vari blocchi (1.2).
 * Un generico client (che sarà associato a una specifica LAN in base al blocco di appartenenza), si collegherà al server Caffe Sbagliato DMI (stabilirà una concessione) ed effettuerà, in modo sequenziale le seguenti operazioni:
 *
 * 1. Il client riceverà (dal server) l'elenco di tutti i prodotti disponibili.
 *    Il client visualizzerà i prodotti secondo il seguente schema:
 *    - `ID prodotto_1, Nome_Prodotto_1, Prezzo_1, Quantità_1`
 *    - `ID prodotto 2, Nome Prodotto_2, Prezzo_2, Quantita_2`
 *    - `...`
 *    - `ID prodotto_N, Nome_Prodotto_N, Prezzo_N, Quantità_N`
 * 2. Il client, invierà la seguente stringa, contenente il prodotto scelto:
 *    - `ID prodotto, quantità`
 * 3. Il server sceglierà in modo casuale sia un prodotto diverso che una quantità differente rispetto quello scelto dall'utente.
 *    Il server restituirà il nome del prodotto erogato e la relativa quantità.
 *    Infine, il server aggiornerà la quantità disponibile per quel prodotto.
 * 4. Il client inserirà il carattere "c" per continuare (in questo caso si riparte dal punto 1), oppure il carattere "q" per concludere l'acquisto (in questo caso la connessione con il server si concluderà).
 *
 * @version 0.1
 * @date 2024-03-07
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>
#include "definitions.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct
{
    int sockfd;
    Product *products;
    sem_t *sem;
} ThreadArgs;

void send_product_list(int sockfd, Product *products)
{
    char buffer[MAX_BUFFER_SIZE];

    // Invio dell'elenco dei prodotti
    printf("Invio dell'elenco dei prodotti\n");
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    for (int i = 0; i < NUM_PRODUCTS; i++)
    {
        snprintf(buffer + strlen(buffer), MAX_BUFFER_SIZE - strlen(buffer) - 1, "%d, %s, %d, %d\n", products[i].product_id, products[i].product_name, products[i].price, products[i].quantity);
    }
    printf("Elenco prodotti:\n%s\n", buffer);
    debug(send(sockfd, buffer, strlen(buffer) + 1, 0));
}

/**
 * @brief Funzione che gestisce la richiesta di un client.
 * Viene lanciata da un thread.
 * @param args Argomenti passati al thread
 */
void *handle_client(void *args)
{
    ThreadArgs *t_args = (ThreadArgs *)args;
    int sockfd = t_args->sockfd;

    char buffer[MAX_BUFFER_SIZE];
    char *token;
    int product_id, quantity;

    send_product_list(sockfd, t_args->products);

    // Ricezione dell'ordine
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    while (recv(sockfd, buffer, MAX_BUFFER_SIZE, 0) > 0)
    {
        product_id = atoi(strtok(buffer, ","));
        quantity = atoi(strtok(NULL, ","));
        printf("Il client ha ordinato %d unità del prodotto con id %d\n", quantity, product_id);

        // Scelta casuale di un prodotto e di una quantità
        int random_product_id, random_quantity;
        srand(time(NULL));
        do
        {
            random_product_id = rand() % NUM_PRODUCTS;
            random_quantity = rand() % t_args->products[random_product_id].quantity;
        } while (random_product_id == product_id || random_quantity == quantity || !t_args->products[random_product_id].quantity);

        // Aggiornamento della quantità disponibile
        sem_wait(t_args->sem);
        t_args->products[random_product_id].quantity -= random_quantity;
        sem_post(t_args->sem);

        // Invio del prodotto scelto
        memset(buffer, '\0', MAX_BUFFER_SIZE);
        snprintf(buffer, MAX_BUFFER_SIZE, "%s, %d", t_args->products[random_product_id].product_name, random_quantity);
        printf("Invio del prodotto scelto (piu o meno): %s\n", buffer);
        debug(send(sockfd, buffer, strlen(buffer) + 1, 0));
        // Invio dell'elenco aggiornato dei prodotti
        send_product_list(sockfd, t_args->products);

        // Pulizia del buffer
        memset(buffer, '\0', MAX_BUFFER_SIZE);
    }

    close(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int sockfd, new_sockfd;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in6);
    sem_t sem;
    Product products[NUM_PRODUCTS] = {
        {0, "Caffè", 50, 10},
        {1, "Cappuccino", 100, 5},
        {2, "Cioccolata", 150, 3},
        {3, "Tè", 200, 2},
    };

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inizializzazione del semaforo
    sem_init(&sem, 0, 1);

    // Creazione del socket
    debug(sockfd = socket(AF_INET6, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin6_addr = in6addr_any;          // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 10));

    // Attesa della connessione dei due giocatori
    ThreadArgs args = {new_sockfd, products, &sem};
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0)
    {
        pthread_t tid;
        args.sockfd = new_sockfd;
        pthread_create(&tid, NULL, handle_client, &args);
    }

    // Chiusura del socket
    close(sockfd);
}
