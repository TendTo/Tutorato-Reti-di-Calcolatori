#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_PORT_SIZE 8
#define MAX_MSG_SIZE 1024
#define MAX_MSG_TO_SERVER_SIZE INET_ADDRSTRLEN + MAX_PORT_SIZE + MAX_MSG_SIZE

/**
 * @brief Il server si occupa di reindirizzare
 * i messaggi ricevuti da un client mittente ad in client destinatario.
 * Il messaggio inviato al server avrà il seguente formato:
 *
 * <ip destinatario> <porta destinatario> <messaggio>
 *
 * Il server invierà SOLO il messaggio al destinatario inteso.
 *
 * La sintassi di avvio del programma è
 *
 * ./server <porta di ascolto sever>
 *
 * @param argc numero di argomenti da riga di comando
 * @param argv lista di argomenti da riga di comando
 * @return codice di uscita
 */
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr, dest_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char msg[MAX_MSG_TO_SERVER_SIZE];

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <porta di ascolto server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    // Settiamo l'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("In attesa di messaggi\n");
        recvfrom(sockfd, msg, MAX_MSG_TO_SERVER_SIZE, 0, (struct sockaddr *)&client_addr, &len);
        char *ip = strtok(msg, " ");
        char *port = strtok(NULL, " ");
        char *message = strtok(NULL, " ");
        printf("Ricevuto un messaggio:\nip: %s, port: %s, message %s\n", ip, port, message);
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(atoi(port));
        inet_pton(AF_INET, ip, &dest_addr.sin_addr);

        sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr *)&dest_addr, len);
    }

    return 0;
}