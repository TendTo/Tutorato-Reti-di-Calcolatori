/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Due macchine client si sfideranno tra di loro a fine di vincere una partita di Forza 4, con una griglia 6x7.
 * Il server dovrà gestire la partita, controllare l'eventuale vincitore e notificare i client sullo stato della partita.
 * Ad ogni turno:
 *
 * - Il server prima invierà al client di turno lo stato del gioco, ovvero la griglia di gioco con le pedine già posizionate.
 * - Ogni client ad ogni turno avrà la possibilità solo di indicare la colonna in cui posizionare la pedina (1 - 7). Se la mossa non è valida, il server dovrà notificare il client e chiedere di ripetere la mossa.
 * - Non appena il giocatore avrà posizionato la pedina, il server dovrà controllare se il giocatore ha vinto o meno. In caso di vittoria, il server dovrà notificare il client e terminare la partita.
 * - Se non è più possibile fare mosse, la partita finisce in pareggio.
 * - Si vince quando si allineano 4 pedine dello stesso colore in orizzontale, verticale o diagonale.
 * - I client dovranno attendere il proprio turno per giocare, probabilmente bloccati da una `recvfrom()`.
 * - Ad ogni mossa, il client indica solo la colonna dove inserire la pedina, che "cadrà" riempendo la prima casella vuota della colonna.
 * - Non è possibile inserire pedine in una colonna già piena.
 *
 * Il server dovrà memorizzare in una struct client il nome del giocatore e tutte le informazioni necessarie a comunicare con il client successivamente.
 * Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.
 * I client seguono il protocollo onestamente.
 * Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).
 *
 * Progettare ed implementare Forza 4 tramite TCP o UDP, considerando l'architettura vista prima.
 * @version 0.1
 * @date 2023-07-12
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 32

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[MAX_BUFFER_SIZE];

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

    if (connect(sockfd, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Inserisci il tuo nome: ");
    fgets(buffer, MAX_NAME_SIZE, stdin);
    buffer[strlen(buffer) - 1] = '\0';
    send(sockfd, buffer, strlen(buffer) + 1, 0);

    while (1)
    {
        recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);

        printf("%s\n", buffer);

        if (strstr(buffer, "Hai vinto!") != NULL || strstr(buffer, "Hai perso!") != NULL || strstr(buffer, "Pareggio!") != NULL)
            break;

        int column;
        do
        {
            printf("Inserisci la colonna: ");
            fgets(buffer, MAX_BUFFER_SIZE, stdin);
            column = atoi(buffer);
            printf("Hai scelto la colonna %d\n", column);
        } while (column <= 0 || column > 7);

        send(sockfd, &column, sizeof(int), 0);
    }

    close(sockfd);

    return 0;
}
