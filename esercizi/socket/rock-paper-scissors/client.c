/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Si richiede di implementare un programma che simuli il gioco della morra cinese in multiplayer.
 * L'architettura sarà composta da un server che si frappone fra due client, che si sfidano ad una partita.
 * Il server si occuperà di gestire le connessioni e di inviare i messaggi ai client.
 * All'inizio del gioco, i due client si connettono al server, che attende la connessione di entrambi.
 * Non appena entrambi sono pronti, il server invia un messaggio ai client, che indica l'inizio del gioco.
 * A questo punto, i due client inviano al server la propria scelta (sasso, carta o forbice).
 * Non è possibile cambiare la propria scelta una volta inviata.
 * Il server verifica chi ha vinto e invia un messaggio ai client, che indica il vincitore.
 * Il punteggio viene anche aggiornato internamente al server.
 * Alla fine del numero di round stabilito, il server invia un messaggio ai client, che indica la fine del gioco.
 * Le connessioni vengono terminate.
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

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
#define MAX_MOVE_SIZE 1
#define CONCLUSION_MSG "La partita si e' conclusa dopo"
#define INVALID_MOVE_MSG "Mossa non valida"

typedef enum
{
    ROCK = 'r',     // Sasso
    PAPER = 'p',    // Carta
    SCISSORS = 's', // Forbice
} Move;

int main(int argc, char const *argv[])
{
    int sockfd, player_idx = 0;
    struct sockaddr_in server_addr;
    pthread_t players_tid[2];
    socklen_t server_addr_len = sizeof(server_addr);
    char msg[MAX_BUFFER_SIZE];
    int n_rounds;

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <server ip> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));      // Porta del server
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo del server

    // Accettazione connessione
    debug(connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len));

    printf("Connessione al server %s:%s effettuata con successo\n", argv[1], argv[2]);

    // Inizio partita
    debug(recv(sockfd, &n_rounds, sizeof(int), 0));
    printf("Avvio di una partita da %d round\n", n_rounds);
    while (n_rounds > 0)
    {
        Move move;

        printf("Inserisci la tua mossa (r = rock, p = paper, s = scissors): ");
        move = getchar();
        while (getchar() != '\n')
            ;
        debug(send(sockfd, &move, sizeof(Move), 0));

        // Attesa del risultato
        debug(recv(sockfd, msg, MAX_BUFFER_SIZE, 0));
        printf("%s\n", msg);

        if (strcmp(msg, INVALID_MOVE_MSG) != 0) // Solo se si è trattata di una mossa valida, si decrementa n_rounds
            n_rounds--;
    }

    // Messaggio finale
    debug(recv(sockfd, msg, MAX_BUFFER_SIZE, 0));
    printf("%s\n", msg);

    // Chiusura del socket
    close(sockfd);

    return 0;
}