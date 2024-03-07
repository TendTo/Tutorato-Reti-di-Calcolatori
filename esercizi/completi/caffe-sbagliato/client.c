/**
 * @file client.c
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
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "definitions.h"

void prompt(char response[MAX_BUFFER_SIZE], const char prompt[])
{
    memset(response, '\0', MAX_BUFFER_SIZE);
    printf(prompt);
    fgets(response, MAX_BUFFER_SIZE, stdin);
    response[strcspn(response, "\n")] = '\0';
}

void show_products(int sockfd)
{
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("Elenco prodotti:\n%s\n", buffer);
}

void order_product(int sockfd)
{
    char id[MAX_BUFFER_SIZE];
    char quantity[MAX_BUFFER_SIZE];
    prompt(id, "Inserisci l'id del prodotto che vuoi ordinare: ");
    prompt(quantity, "Inserisci la quantità del prodotto che vuoi ordinare: ");
    printf("Invio ordine per %s unità del prodotto con id %s\n", quantity, id);
    strncat(id, ",", MAX_BUFFER_SIZE - strlen(id) - 1);
    strncat(id, quantity, MAX_BUFFER_SIZE - strlen(id) - 1);
    debug(send(sockfd, id, strlen(id) + 1, 0));

    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("Il server ha risposto con: %s\n", buffer);
    memset(buffer, '\0', MAX_BUFFER_SIZE);
    debug(recv(sockfd, buffer, MAX_BUFFER_SIZE, 0));
    printf("Elenco prodotti:\n%s\n", buffer);
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in6 server_addr;
    socklen_t len = sizeof(struct sockaddr_in6);

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[2]));
    inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);

    debug(connect(sockfd, (struct sockaddr *)&server_addr, len));

    show_products(sockfd); // visualizzazione prodotti
    order_product(sockfd); // invio ordine

    while (1)
    {
        printf("Che operazione vuoi effettuare? [c|q]\n");
        char action = getchar();
        while (getchar() != '\n')
            ;
        if (action != CONTINUE && action != QUIT) // input non valido
            continue;
        if (action == QUIT) // chiusura client
            break;
        order_product(sockfd); // invio ordine
    }

    close(sockfd);
    return 0;
}
