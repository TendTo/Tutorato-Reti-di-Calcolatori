/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Due macchine client si sfideranno tra di loro a fine di indovinare la parola segreta presente in una macchina server.
 * Ogni macchina client avrà a disposizione 3 vite. Il conteggio delle vite sarà tenuto dal server.
 * Ad ogni turno:
 *
 * - Il server prima invierà al client di turno lo stato del gioco: lettere indovinate, lettere mancanti e struttura della parola
 * - Ogni client ad ogni turno avrà la possibilità solo di scrivere da riga di comando o una lettere o dare la risposta completa.
 *   In caso di errore, perderà una vita.
 *   Il server notificherà allo specifico client l'esito del turno.
 * - Se il giocatore di turno perde tutte le vite, dovrà terminare l'esecuzione del programma.
 *
 * Il server dovrà memorizzare in una struct client il nome del giocatore, l'indirizzo IP e la porta del client.
 * Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.
 * Il server sceglierà randomicamente una stringa da utilizzare per il gioco dell'impiccato tra quelle presenti in un array bidimensionale.
 * Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).
 * Progettare ed implementare **L'impiccato!** tramite socket UDP, considerando l'architettura vista prima.
 * @version 0.1
 * @date 2023-07-10
 *
 * @copyright Copyright (c) 2023
 *
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

    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    perror("socket");

    printf("Inserire nome: ");
    fflush(stdout);
    memset(buffer, 0, MAX_BUFFER_SIZE);
    fgets(buffer, MAX_NAME_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    printf("Connessione al server: io sono '%s'\n", buffer);
    sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&server_addr, len);
    perror("sendto username");

    while (1)
    {
        // Interpellato dal server
        memset(buffer, 0, MAX_BUFFER_SIZE);
        recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len);
        perror("recvfrom turno");

        printf("%s\n", buffer);

        if (strstr(buffer, "partita finita"))
            break;

        printf("Inserisci una lettera o prova ad indovinare la parola\n");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        printf("Hai scelto di inviare la %s %s\n", strlen(buffer) == 1 ? "lettera" : "parola", buffer);

        sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&server_addr, len);
        perror("sendto parola");
    }

    close(sockfd);

    return 0;
}
