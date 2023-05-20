/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Semplice server tcp ipv4
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

#define SERVER_RESP "Server's answer: "
#define MAX_MSG_SIZE 1000
#define PORT 2000
#define N_ACCEPT 5

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char server_message[MAX_MSG_SIZE], client_message[MAX_MSG_SIZE];
    socklen_t client_struct_length = sizeof(client_addr);
    size_t strl = strlen(SERVER_RESP);

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Creazione del socket.
    // Dato che upd è il protocollo di default per SOCK_STREAM, il terzo parametro può essere 0
    // sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error while creating socket");
        return 1;
    }
    printf("Socket created successfully\n");

    // Si impostano le informazioni del server
    // In questo esempio la porta è hard-coded
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Il server accetta connessioni da qualsiasi indirizzo

    // Si associa il socket all'indirizzo e alla porta
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Couldn't bind to the port");
        return -1;
    }
    printf("Done with binding\n");

    // Si è disposti ad accettare le richieste di connessione in arrivo
    if (listen(sockfd, N_ACCEPT) < 0)
    {
        perror("Could't listen to connections");
        return 1;
    }

    int sockfd_conn = accept(sockfd, (struct sockaddr *)&client_addr, &client_struct_length);
    if (sockfd_conn < 0)
    {
        perror("Couldn'd accept new connection");
        return 1;
    }
    printf("Listening for incoming messages...\n\n");
    if (recv(sockfd_conn, client_message, sizeof(client_message), 0) < 0)
    {
        perror("Couldn't receive");
        return -1;
    }
    printf("Received message from IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    printf("Msg from client: %s\n", client_message);

    // Si prepara il messaggio da inviare al client
    strcat(server_message, SERVER_RESP);
    strncat(server_message + strl, client_message, sizeof(client_message) - strl);

    // Si invia il messaggio al client
    if (send(sockfd_conn, server_message, strlen(server_message), 0) < 0)
    {
        perror("Can't send");
        return 1;
    }

    // Si chiude il socket
    close(sockfd_conn);
    close(sockfd);

    return 0;
}
