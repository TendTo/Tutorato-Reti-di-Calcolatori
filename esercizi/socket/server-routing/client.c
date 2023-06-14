/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Servizio che utilizza un server di routing per inviare messaggi ad un altro client
 * All'avvio del programma, verrà indicato al client su quale porta dovrà mettersi in ascolto (che dovrebbe essere uguale per tutti i client)
 * e l'indirizzo IP e la porta del server di routing.
 * Nel momento in cui vuole inoltrare un messaggio, il mittente dovrà specificare il destinatario e il messaggio.
 * Questo verrà inviato al server di routing che si occuperà di inoltrarlo al client destinatario.
 * @version 0.1
 * @date 2023-06-14
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MSG_SIZE 2000

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_len = sizeof(server_addr);
    socklen_t client_len = sizeof(client_addr);
    // Il buffer deve essere abbastanza grande da contenere l'indirizzo IP del client destinatario, uno spazio e il messaggio che si vuole inviare
    // Esempio:
    // 10.0.0.1 Ciao come stai?
    // 192.168.1.2 Bene, tu?
    char buffer[INET_ADDRSTRLEN + MAX_MSG_SIZE + 1];

    if (argc < 4 || atoi(argv[1]) == 0 || atoi(argv[3]) == 0)
    {
        fprintf(stderr, "use: %s <porta d'ascolto client> <ip server> <porta server>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, server_len);
    memset(&client_addr, 0, client_len);
    memset(buffer, '\0', sizeof(buffer));

    // Si impostano le informazioni del server a cui connettersi
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[3]));
    server_addr.sin_addr.s_addr = inet_addr(argv[2]);

    // Si imposta la socket su cui il client riceverà i messaggi provenienti dal server
    // Per assicurare che solo il server possa inviare messaggi,
    // si imposta l'indirizzo a quello del server invece usare INADDR_ANY
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(atoi(argv[1]));
    client_addr.sin_addr.s_addr = INADDR_ANY;

    int pid = fork();

    if (pid == 0) // Processo figlio
    {
        // Creazione del socket del figlio.
        // Dato che upd è il protocollo di default per SOCK_DGRAM, il terzo parametro può essere 0 e verrà usato udp
        // sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("Error while creating socket");
            return EXIT_FAILURE;
        }

        // Il client effettua la bind per ricevere i messaggi dal server
        if (bind(sockfd, (struct sockaddr *)&client_addr, client_len) < 0)
        {
            perror("Error while binding socket");
            return EXIT_FAILURE;
        }

        while (1)
        {
            // Si riceve il messaggio dal server
            if (recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr *)&server_addr, &server_len) < 0)
            {
                perror("Error while receiving server's msg");
                return EXIT_FAILURE;
            }

            // Si processa il messaggio ricevuto, separando l'indirizzo IP del mittente dal messaggio
            // e il messaggio stesso, spezzando la stringa
            printf("Messaggio ricevuto: %s\n", buffer);
        }
    }
    else if (pid > 0)
    {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("Error while creating socket");
            return EXIT_FAILURE;
        }

        // Il client invia i messaggi al server
        while (1)
        {
            // Prendiamo l'input dell'utente e lo si salva nel buffer client_message
            printf("A quale IP vuoi inviare il messaggio? ");
            fgets(buffer, INET_ADDRSTRLEN, stdin);
            // Rimuoviamo il carattere '\n' dalla stringa, sostiutendolo con uno spazio che servirà come separatore
            // tra l'indirizzo IP e il messaggio
            int space_idx = strcspn(buffer, "\n");
            buffer[space_idx] = ' ';
            printf("Che messaggio vuoi inviare? ");
            fgets(buffer + space_idx + 1, MAX_MSG_SIZE, stdin);
            // Rimuoviamo il carattere '\n' dalla stringa
            buffer[strcspn(buffer, "\n")] = '\0';

            // Si invia il messaggio al server
            if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, server_len) < 0)
            {
                perror("Unable to send message");
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        perror("Error while forking");
        return EXIT_FAILURE;
    }

    // Si chiude il socket
    close(sockfd);
    return EXIT_SUCCESS;
}
