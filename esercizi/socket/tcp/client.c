/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Semplice client tcp ipv4
 * @version 0.1
 * @date 2023-05-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MSG_SIZE 2000
#define PORT 2000

int main(int argc, char* argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    char server_message[MAX_MSG_SIZE], client_message[MAX_MSG_SIZE];
    socklen_t server_struct_length = sizeof(server_addr);

    if (argc < 2) {
        fprintf(stderr, "use: %s address\n", argv[0]);
        return 1;
    }

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, server_struct_length);
    memset(server_message, 0, sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Creazione del socket.
    // Dato che tcp è il protocollo di default per SOCK_STREAM, il terzo parametro può essere 0
    // sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error while creating socket");
        return 1;
    }
    printf("Socket created successfully\n");

    // Si impostano le informazioni del server a cui connettersi
    // In questo esempio porta e indirizzo sono hard-coded
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Prendiamo l'input dell'utente e lo si salva nel buffer client_message
    printf("Enter message: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Si apre una connessione al server
    if (connect(sockfd, (struct sockaddr *)&server_addr, server_struct_length) < 0)
    {
        perror("Unable to connect");
        return 1;
    }

    // Si invia il messaggio al server
    if (send(sockfd, client_message, strlen(client_message), 0) < 0)
    {
        perror("Unable to send message");
        return 1;
    }

    // Si riceve la risposta del server
    if (recv(sockfd, server_message, sizeof(server_message), 0) < 0)
    {
        perror("Error while receiving server's msg");
        return 1;
    }

    printf("Server's response: %s\n", server_message);

    // Si chiude il socket
    close(sockfd);

    return 0;
}
