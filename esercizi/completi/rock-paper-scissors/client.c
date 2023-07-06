/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare un programma scritto in c (o c++) che simuli una partita a "Carta-Sasso-Forbice" fra i due Client, con il Server in mezzo a fare da arbitro.
 * Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.
 * Appena avviato, ogni Client si connette al Server per registrarsi, fornendo anche le informazioni che il Server userà per comunicare durante la partita (nome, ip, porta).
 * Quando entrambi i Client si sono registrati, il Server avvia la partita, impostando il numero di vite di entrambi i giocatori ad un valore stabilito.
 * Inizia quindi ad interpellare a turno i due giocatori, chiedendo la loro mossa.
 * Quando entrambi hanno giocato, il Server comunica il risultato della partita a entrambi i giocatori.
 * Se non si è verificato un pareggio, toglie una vita a quello che ha perso e ricomincia il ciclo.
 * Quando un giocatore rimane senza vite, il Server comunica il vincitore ad entrambi e termina la partita.
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
#define REQ_MOVE "Fai la tua mossa"

typedef enum
{
    ROCK = 'r',     // Sasso
    PAPER = 'p',    // Carta
    SCISSORS = 's', // Forbice
} Move;

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    char msg[MAX_BUFFER_SIZE];

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <server ip> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));      // Porta del server
    server_addr.sin_addr.s_addr = inet_addr(argv[1]); // Indirizzo del server

    debug(sendto(sockfd, NULL, 0, 0, (struct sockaddr *)&server_addr, server_addr_len));
    printf("Connessione al server %s:%s effettuata con successo\n", argv[1], argv[2]);

    // Inizio partita
    printf("Avvio della partita\n");
    while (1)
    {
        Move move;

        // Attesa per essere interpellati dal server
        debug(recvfrom(sockfd, msg, sizeof(REQ_MOVE), 0, (struct sockaddr *)&server_addr, &server_addr_len));

        printf("Inserisci la tua mossa (r = rock, p = paper, s = scissors): ");
        move = getchar();
        while (getchar() != '\n')
            ;

        debug(sendto(sockfd, &move, sizeof(Move), 0, (struct sockaddr *)&server_addr, server_addr_len));

        // Attesa del risultato
        debug(recvfrom(sockfd, msg, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_addr_len));
        printf("%s\n", msg);

        // Se la stringa contiene la frase "vinto la partita" allora la partita è terminata
        if (strstr(msg, "vinto la partita") != NULL)
            break;
    }

    // Chiusura del socket
    close(sockfd);

    return 0;
}