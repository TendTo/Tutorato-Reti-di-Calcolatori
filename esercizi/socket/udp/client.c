/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Semplice client udp ipv4
 * @version 0.1
 * @date 2023-05-11
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
#define IP_ADDR "192.168.56.101"

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[MAX_MSG_SIZE], client_message[MAX_MSG_SIZE];
    socklen_t server_struct_length = sizeof(server_addr);

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, server_struct_length);
    memset(server_message, 0, sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Creazione del socket.
    // Dato che upd è il protocollo di default per SOCK_DGRAM, il terzo parametro può essere 0
    // socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
    {
        perror("Error while creating socket");
        return 1;
    }
    printf("Socket created successfully\n");

    // Si impostano le informazioni del server
    // In questo esempio porta e indirizzo sono hard-coded
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    // Prendiamo l'input dell'utente e lo si salva nel buffer client_message
    printf("Enter message: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Si invia il messaggio al server
    if (sendto(socket_desc, client_message, strlen(client_message), 0,
               (struct sockaddr *)&server_addr, server_struct_length) < 0)
    {
        perror("Unable to send message");
        return 1;
    }

    // Si riceve la risposta del server
    if (recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                 (struct sockaddr *)&server_addr, &server_struct_length) < 0)
    {
        perror("Error while receiving server's msg");
        return 1;
    }

    printf("Server's response: %s\n", server_message);

    // Si chiude il socket
    close(socket_desc);

    return 0;
}
