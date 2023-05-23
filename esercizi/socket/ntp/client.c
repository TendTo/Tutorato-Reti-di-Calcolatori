/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Client del protocollo NTP.
 * Il client si registra al server inviandogli una richiesta.
 * Da quel momento in poi, per 30 secondi, riceverà un messaggio con data e ore correnti ogni 5 secondi.
 * Trascorsi i 30 secondi, la registrazione dovrà essere rinnovata per poter usufruire nuovamente del servizio.
 * @version 0.1
 * @date 2023-05-22
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_LEN 100
#define RESP_DEL "Servizio terminato"

int main(int argc, char const *argv[])
{
    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s address port", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    char buffer[MAX_BUFFER_LEN];
    char *server_resp;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    memset(&server_addr, 0, addr_len);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // Il primo messaggio può essere qualsiasi cosa, basta che il server lo riceva
    if (sendto(sockfd, "Fest", 5, 0, (struct sockaddr *)&server_addr, addr_len) < 0)
    {
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Attende le risposte del server
        if (recvfrom(sockfd, buffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&server_addr, &addr_len) < 0)
        {
            perror("recvfrom()");
            exit(EXIT_FAILURE);
        }
        printf("Dal server: %s\n", buffer);

        // Il ciclo si conclude quando il servizio termina
        if (strcmp(buffer, RESP_DEL) == 0)
        {
            break;
        }
    }

    close(sockfd);

    return 0;
}
