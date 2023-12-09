/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Gli studenti di informatica del corso di Reti di Calcolatori hanno deciso di implementare un server Christmas in grado di ricevere e gestire bigliettini di Natale contenenti messaggi del tipo "Oggi supererò il laboratorio di Reti".
 * Un generico client avrà la possibilità di:
 *
 * - inviare un messaggio al server
 * - fare una richiesta per ricevere tutti i messaggi già inviati (solo dello specifico client).
 * - modificare e/o cancellare un messaggio specifico.
 *
 * Il server dovrà essere progettato in modo da memorizzare qualsiasi messaggio (associandolo allo specifico client) e gestire le operazioni precedentemente elencate.
 * Quando il client invierà il carattere `q`, il programma (per lo specifico client) terminerà.
 *
 * Si utilizzino le Socket UDP per la comunicazione.
 *
 * Lo studente può definire qualsiasi approccio per risolvere l'esercizio.
 * @version 0.1
 * @date 2023-12-08
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "definitions.h"

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    Message msg;

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    debug(memset(&server_addr, 0, len));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Decide quale azione effettuare. Inserimento, aggiornamento, eliminazione, lista o uscita
        Action action;
        printf("Che operazione vuoi effettuare? [i|u|d|l|q]\n");
        action = getchar();
        while (getchar() != '\n')
            ;

        // Se l'azione non è valida, ripete il ciclo
        if (action != INSERT && action != UPDATE && action != DELETE && action != LIST && action != QUIT)
            continue;

        // Se l'azione è uscita, termina il programma
        if (action == QUIT)
        {
            printf("Termino il programma\n");
            break;
        }

        // Crea il messaggio da inviare al server
        Message msg;
        memset(&msg, 0, sizeof(msg));
        msg.action = action;

        // Se c'è da modificare o eliminare un messaggio, chiede l'id del messaggio da modificare/eliminare.
        if (action == DELETE || action == UPDATE)
        {
            printf("Inserisci l'id del messaggio da modificare/eliminare: ");
            fgets(msg.message, MAX_BUFFER_SIZE, stdin);
            msg.message[strcspn(msg.message, "\n")] = '\0';
            msg.id = atoi(msg.message);
        }

        // Se c'è da richiedere la lista di messaggi del client, chiede l'id del client.
        // Si può anche usare 0 per richiedere la lista di messaggi del client che ha fatto la richiesta
        if (action == LIST)
        {
            printf("Inserisci l'id del client a cui sei interessato o 0 per indicare te stesso: ");
            fgets(msg.message, MAX_BUFFER_SIZE, stdin);
            msg.message[strcspn(msg.message, "\n")] = '\0';
            msg.id = atol(msg.message);
        }

        // Se c'è da inserire o modificare un messaggio, chiede il messaggio da inviare al server
        // Il messaggio deve essere non vuoto
        memset(msg.message, 0, MAX_BUFFER_SIZE);
        if ((action == INSERT || action == UPDATE) && strlen(msg.message) == 0)
        {
            printf("Inserisci il nuovo messaggio: ");
            fgets(msg.message, MAX_BUFFER_SIZE, stdin);
            msg.message[strcspn(msg.message, "\n")] = '\0';
        }

        debug(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, len));

        if (action == LIST)
        {
            printf("Elenco messaggi:\n");
            while (1)
            {
                debug(recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, &len));
                if (msg.action == END)
                    break;
                if (msg.action == FAIL)
                {
                    printf("Il server non è riuscito ad adempiere alla richiesta\n");
                    break;
                }

                printf("id: %d, message: %s\n", msg.id, msg.message);
            }
        }
        else
        {
            debug(recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, &len));
            printf("Il server ha %s la richiesta\n", msg.action == SUCCESS ? "completato" : "fallito");
        }
    }

    close(sockfd);
    return 0;
}
