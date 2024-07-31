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
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "definitions.h"

/**
 * @brief Mostra un @p prompt all'utente e salva la risposta in @p response.
 * @note La funzione rimuove il carattere di newline finale dalla stringa.
 * @pre @p response deve essere in grado di contenere almeno @c MAX_BUFFER_SIZE caratteri.
 * @param response[out] risposta scritta dall'utente
 * @param prompt messaggio da mostrare all'utente
 */
void prompt(char response[MAX_BUFFER_SIZE], const char prompt[])
{
    memset(response, '\0', MAX_BUFFER_SIZE);
    printf("%s", prompt);
    fgets(response, MAX_BUFFER_SIZE, stdin);
    response[strcspn(response, "\n")] = '\0';
}

/**
 * @brief Autentica l'utente con il server.
 * @param sockfd file descriptor del socket
 * @param request richiesta di autenticazione
 */
void authenticate(int sockfd, Request *const request)
{
    char buffer[MAX_BUFFER_SIZE];

    prompt(request->data, "Inserisci il tuo nome: ");
    debug(send(sockfd, request, sizeof(Request), 0));

    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("Risposta del server: %s\n", buffer);
}

/**
 * @brief Mostra l'elenco dei prodotti disponibili.
 * @param sockfd file descriptor del socket
 * @param request richiesta di visualizzazione prodotti
 */
void show_products(int sockfd, Request *const request)
{
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(send(sockfd, request, sizeof(Request), 0));
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("%s\n", buffer);
}

/**
 * @brief Ordina un prodotto specificando l'id e la quantità.
 * @param sockfd file descriptor del socket
 */
void order_product(int sockfd, Request *const request)
{
    char quantity[MAX_BUFFER_SIZE];
    prompt(request->data, "Inserisci l'id del prodotto che vuoi ordinare: ");
    prompt(quantity, "Inserisci la quantità del prodotto che vuoi ordinare: ");
    printf("Invio ordine per %s unità del prodotto con id %s\n", quantity, request->data);
    strncat(request->data, ",", MAX_BUFFER_SIZE - strlen(request->data) - 1);
    strncat(request->data, quantity, MAX_BUFFER_SIZE - strlen(request->data) - 1);
    debug(send(sockfd, request, sizeof(Request), 0));

    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("Il server ha risposto con: %s\n", buffer);
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    debug(connect(sockfd, (struct sockaddr *)&server_addr, len));

    Request request;
    memset(&request, 0, sizeof(Request));
    request.action = AUTHENTICATE;
    authenticate(sockfd, &request); // autenticazione

    printf("Inserisci 'q' per uscire, 'o' per ordinare un prodotto, 'l' per visualizzare la lista dei prodotti\n");
    while (1)
    {
        memset(&request.data, 0, MAX_BUFFER_SIZE);
        printf("Che operazione vuoi effettuare? [o|l|q]\n");
        char action = getchar();
        while (getchar() != '\n')
            ;
        request.action = action;
        if (action == QUIT) // chiusura client
            break;
        else if (action == ORDER)
            order_product(sockfd, &request); // invio ordine
        else if (action == LIST)
            show_products(sockfd, &request); // visualizzazione prodotti
    }

    close(sockfd);
    return 0;
}
