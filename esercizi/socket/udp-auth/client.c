/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Client in grado di connettersi con il server di autenticazione.
 * L'utente dovrà inserire il proprio username e password per registrarsi, a patto che questi non siano già presenti.
 * Utilizza ipv6.
 * @version 0.1
 * @date 2023-05-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20
#define MAX_MSG NAX_USERNAME_SIZE + MAX_PASSWORD_SIZE

typedef struct
{
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
} User;

typedef enum
{
    REGISTER = 'r',
    LOGIN = 'l',
    DELETE = 'd',
    EXIT = 'e'
} Operation;

typedef struct
{
    Operation op;
    User u;
} Message;

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in6 server_addr;
    socklen_t sock_len = sizeof(struct sockaddr_in6);
    char buffer[1000];

    if (argc < 3)
    {
        printf("Use: %s udp_server_ipv6 listening_PORT", argv[0]);
        return 1;
    }

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error in socket opening ... exit!");
        return 1;
    }

    memset(&server_addr, 0, sock_len);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[2]));
    inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);

    while (1)
    {
        // Operazione da effettuare
        Message msg;
        printf("Inserisci operazione da effettuare:\nr: registrazione\nl: login\nd: delete\ne: exit\n");
        msg.op = getchar();
        getchar();

        switch (msg.op)
        {
        case REGISTER:
        case LOGIN:
        case DELETE:
            // Domanda username e password all'utente
            printf("Inserisci username: ");
            fgets(msg.u.username, sizeof(msg.u.username), stdin);
            msg.u.username[strcspn(msg.u.username, "\n")] = '\0';

            printf("Inserisci password: ");
            fgets(msg.u.password, sizeof(msg.u.password), stdin);
            msg.u.password[strcspn(msg.u.password, "\n")] = '\0';

            // Invia username e password
            if (sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sock_len) < 0)
            {
                perror("Error in sending data!");
                return 1;
            }

            // Riceve la risposta dal server
            if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &sock_len) < 0)
            {
                perror("Error in receiving data!");
                return 1;
            }
            printf("Risposta dal server: %s\n", buffer);

            break;
        case EXIT:
            printf("Arrivederci!\n");
            close(sockfd);
            return 0;
        default:
            printf("%d\n", (int)msg.op);
        }
    }

    return 0;
}