/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare un programma scritto in c (o c++) che simuli una partita a "Tris" fra i due Client, con il Server in mezzo a fare da arbitro.
 * Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.
 * Appena avviato, ogni Client si connette al Server per registrarsi, fornendo anche le informazioni che il Server userà per comunicare durante la partita (ip, porta).
 * Quando entrambi i Client si sono registrati, il Server avvia la partita.
 * Inizia quindi ad interpellare a turno i due giocatori, chiedendo dove porre il loro simbolo sulla griglia.
 * Dopo ogni mossa, il Server risponde inviando la griglia aggiornata.
 * Quando un giocatore ha vinto, il Server invia un messaggio di vittoria al vincitore e di sconfitta al perdente, quindi termina la partita.
 * @version 0.1
 * @date 2023-07-09
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>

#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                  \
    do                                              \
    {                                               \
        int __res__ = fun;                          \
        if (__res__ < 0)                            \
        {                                           \
            error_at_line(EXIT_FAILURE, errno,      \
                          __FILE__, __LINE__, "%s", \
                          #fun);                    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    } while (0)
#else
#define debug(fun) fun
#endif

#define MAX_BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in6 server_addr;
    socklen_t server_addr_len = sizeof(struct sockaddr_in6);
    char msg[MAX_BUFFER_SIZE];

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <server ipv6> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET6, SOCK_STREAM, 0));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[2]));                // Porta del server
    debug(inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr)); // Indirizzo del server
    // Bisogna fare l'associazione con l'interfaccia di rete
    server_addr.sin6_scope_id = if_nametoindex("enp0s3");

    debug(connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len));
    printf("Connessione al server %s:%s effettuata con successo\n", argv[1], argv[2]);

    // Inizio partita
    printf("Avvio della partita\n");
    while (1)
    {
        int n;
        // Attesa per essere interpellati dal server, che invia la griglia aggiornata
        // ed eventuale comunicazione del vincitore
        debug(n = recv(sockfd, msg, MAX_BUFFER_SIZE, 0));
        msg[n] = '\0';
        if (n == 0)
            break;

        printf("%s\n", msg);

        // Se la stringa contiene la frase "vinto la partita" allora la partita è terminata
        if (strstr(msg, "vinto la partita") != NULL)
            break;

        int move;
        do
        {
            printf("Dove vuoi mettere il tuo simbolo?\n");
            printf("| 1 | 2 | 3 |\n");
            printf("| 4 | 5 | 6 |\n");
            printf("| 7 | 8 | 9 |\n");
            fgets(msg, MAX_BUFFER_SIZE, stdin);
            move = atoi(msg);
            printf("Hai scelto la mossa %d\n", move);
        } while (move < 1 || move > 9);

        printf("Invio la mossa al server e attesa della mossa dell'altro giocatore\n");
        debug(send(sockfd, &move, sizeof(int), 0));
    }

    // Chiusura del socket
    close(sockfd);

    return 0;
}