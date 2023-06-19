#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_PORT_SIZE 8
#define MAX_MSG_SIZE 1024
#define MAX_MSG_TO_SERVER_SIZE INET_ADDRSTRLEN + MAX_PORT_SIZE + MAX_MSG_SIZE

void padre(int sockfd, struct sockaddr_in *server_addr)
{
    while (1)
    {
        char ip[INET_ADDRSTRLEN],
            message[MAX_MSG_SIZE],
            port[MAX_PORT_SIZE],
            msg_to_server[MAX_MSG_TO_SERVER_SIZE];

        printf("Qual è l'ip del client a cui vuoi mandare un messaggio?\n");
        fgets(ip, INET_ADDRSTRLEN, stdin);
        ip[strlen(ip) - 1] = '\0';

        printf("Su quale porta ascolterà il client destinatario?\n");
        fgets(port, MAX_PORT_SIZE, stdin);
        port[strlen(port) - 1] = '\0';

        printf("Quale messaggio vuoi inviare?\n");
        fgets(message, MAX_MSG_SIZE, stdin);
        message[strlen(message) - 1] = '\0';

        sprintf(msg_to_server, "%s %s %s", ip, port, message);

        if (sendto(sockfd,
                   msg_to_server,
                   strlen(msg_to_server) + 1,
                   0,
                   (struct sockaddr *)server_addr,
                   sizeof(struct sockaddr_in)) < 0)
            perror("sendto");
    }
}

void figlio(int sockfd)
{
    char message[MAX_MSG_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    while (1)
    {
        printf("In attesa di messaggi\n");
        recvfrom(sockfd, message, MAX_MSG_SIZE, 0, (struct sockaddr *)&sender_addr, &len);
        printf("Messaggio ricevuto: %s\n", message);

        if (strcmp(message, "exit") == 0)
            return;
    }
}

/**
 * @brief Il client vuole parlare con un altro client.
 * Invece di comunicare direttamente, avvia una connessione al suo server di riferimento.
 * Questo si occuperà di ridirigere il messaggio al client destinatario.
 * Il messaggio inviato al server avrà il seguente formato:
 *
 * <ip destinatario> <porta destinatario> <messaggio>
 *
 * La sintassi di avvio del programma è
 *
 * ./client <porta di ascolto client> <ip server di riferimento> <porta server di riferimento>
 *
 * @param argc numero di argomenti da riga di comando
 * @param argv lista di argomenti da riga di comando
 * @return codice di uscita
 */
int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    if (argc < 4 || atoi(argv[1]) == 0 || atoi(argv[3]) == 0)
    {
        fprintf(stderr, "use: %s <porta di ascolto client> <ip server di riferimento> "
                        "<porta server di riferimento>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = client_addr.sin_family = AF_INET;

    // Settiamo l'indirizzo del server
    inet_pton(AF_INET, argv[2], &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[3]));

    // Settiamo l'indirizzo del client in ascolto
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd, (struct sockaddr *)&client_addr, len) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    int pid = fork();

    if (pid == 0) // figlio
    {
        figlio(sockfd);
        close(sockfd);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0) // padre
    {
        padre(sockfd, &server_addr);
        close(sockfd);
    }
    else
    {
        perror("fork()");
        exit(EXIT_FAILURE);
    }

    return 0;
}