#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in remote_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[100];

    if (argc < 3)
    {
        perror("Uso: <IP> <PORTA>");
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Errore apertura socket");
        return -1;
    }

    memset(&remote_addr, 0, len);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(argv[1]);
    remote_addr.sin_port = htons(atoi(argv[2]));

    while (fgets(buffer, 100, stdin) != NULL)
    {
        int n_pos = strcspn(buffer, "\n");
        buffer[n_pos] = '\0'; // rimuove il carattere '\n' dalla stringa
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&remote_addr, len);

        int n = recvfrom(sockfd, buffer, 99, 0, (struct sockaddr *)&remote_addr, &len);

        if (n < 0)
            perror("Errore nella recvfrom");

        buffer[n] = 0;
        printf("Risposta Server: %s, IP server: %s, Porta Server: %d\n",
               buffer,
               inet_ntoa(remote_addr.sin_addr),
               ntohs(remote_addr.sin_port));

        if (strcmp(buffer, "fine") == 0)
            break;
    }

    close(sockfd);
    return 0;
}
