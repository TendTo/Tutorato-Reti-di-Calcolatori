/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Gli studenti del corso di Reti di Calcolatori hanno deciso di implementare un tool per l'impresa "Cocco Bello" al fine di ottimizzare le vendite nelle spiagge libere di Catania.
 * L'impresa metterà a disposizione una sequenza di prodotti con prezzo e quantità a disposizione.
 * Un generico cliente richiederà la lista dei prodotti disponibili e potrà avere la possibilità di acquistare uno o piu prodotti specificandone la quantità.
 * Dopo l'acquisto di un prodotto, la relative quantità verrà aggiornata.
 * Il servizio deve prevedere la possibilità autenticare un utente prima dell'acquisto di un prodotto (_non necessariamente deve essere implementata la funzione di registrazione_).
 * @version 0.1
 * @date 2024-07-31
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
    int num_products;
    char user[MAX_NAME_SIZE];
    sem_t *sem;
} ThreadArgs;

void authenticate(int sockfd, const Request *const request, ThreadArgs *const args)
{
    char buffer[MAX_BUFFER_SIZE];

    // Autenticazione dell'utente
    strncpy(args->user, request->data, MAX_NAME_SIZE);

    printf("Autenticazione dell'utente %s\n", args->user);
    snprintf(buffer, MAX_BUFFER_SIZE, "Benvenuto %s", args->user);
    debug(send(sockfd, buffer, strlen(buffer) + 1, 0));
}

void send_product_list(int sockfd, const ThreadArgs *const args)
{
    char buffer[MAX_BUFFER_SIZE];
    const Product *const products = args->products;

    // Invio dell'elenco dei prodotti
    printf("Invio dell'elenco dei prodotti\n");
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    sprintf(buffer, "Utente %s: Elenco prodotti:\nID, NOME, PREZZO, DISPONIBILI\n", args->user);
    for (int i = 0; i < args->num_products; i++)
    {
        snprintf(buffer + strlen(buffer), MAX_BUFFER_SIZE - strlen(buffer) - 1, "%d, %s, %d, %d\n", products[i].product_id, products[i].product_name, products[i].price, products[i].quantity);
    }
    printf("Utente %s: Elenco prodotti:\n%s", args->user, buffer);
    debug(send(sockfd, buffer, strlen(buffer) + 1, 0));
}

void ordered_product(int sockfd, Request *const request, ThreadArgs *const args)
{
    char buffer[MAX_BUFFER_SIZE];
    Product *const products = args->products;

    int product_id = atoi(strtok(request->data, ","));
    int quantity = atoi(strtok(NULL, ","));
    printf("Il client ha ordinato %d unità del prodotto con id %d\n", quantity, product_id);

    if (product_id < 0 || product_id >= args->num_products)
    {
        printf("Il prodotto con id %d non esiste\n", product_id);
        debug(send(sockfd, "Prodotto non esistente", strlen("Prodotto non esistente") + 1, 0));
        return;
    }
    if (quantity > products[product_id].quantity)
    {
        printf("Il prodotto con id %d non è disponibile in quella quantità (disponibili %d)\n", product_id, products[product_id].quantity);
        debug(send(sockfd, "Prodotto non disponibile", strlen("Prodotto non disponibile") + 1, 0));
        return;
    }

    // Aggiornamento della quantità disponibile
    products[product_id].quantity -= quantity;

    // Invio del prodotto scelto
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    snprintf(buffer, MAX_BUFFER_SIZE, "Utente %s, in arrivo per te %s X %d", args->user, products[product_id].product_name, quantity);
    printf("Utente %s: Invio del prodotto scelto: %s\n", args->user, buffer);
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

    Request request;
    char *token;
    int product_id, quantity;

    // Ricezione dell'ordine
    memset(&request, 0, sizeof(Request));
    while (recv(sockfd, &request, sizeof(Request), 0) > 0)
    {
        printf("Richiesta ricevuta: %c - %s\n", request.action, request.data);
        sem_wait(t_args->sem); // Zona critica: modifica e lettura della lista dei prodotti
        switch (request.action)
        {
        case LIST:
            send_product_list(sockfd, t_args);
            break;
        case ORDER:
            ordered_product(sockfd, &request, t_args);
            break;
        case AUTHENTICATE:
            authenticate(sockfd, &request, t_args);
            break;
        default:
            break;
        }
        sem_post(t_args->sem);
    }

    close(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int sockfd, new_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    sem_t sem;
    Product products[NUM_PRODUCTS] = {
        {0, "Cocco bello", 50, 10},
        {1, "Cocco bellissimo", 100, 5},
        {2, "Cocchino bellino", 150, 3},
        {3, "Limone", 200, 2},
    };

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inizializzazione del semaforo
    sem_init(&sem, 0, 1);

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 10));

    // Attesa della connessione dei due giocatori
    ThreadArgs args = {new_sockfd, products, NUM_PRODUCTS, "\0", &sem};
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0)
    {
        printf("Connessione accettata\n");
        pthread_t tid;
        args.sockfd = new_sockfd;
        pthread_create(&tid, NULL, handle_client, &args);
    }

    // Chiusura del socket
    close(sockfd);
}
