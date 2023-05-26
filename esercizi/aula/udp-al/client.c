#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    int sockfd, datalen;
    char buffer[2000];
    struct sockaddr_in server_addr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    extern int errno;

    if (argc != 3)
    {
        perror("ERRORE. './client <IP> <DESTINATION_PORT>'");
        return errno;
    }

    memset(buffer, 0, strlen(buffer));
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("ERRORE nell'apertura della socket");
        return errno;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    for (;;)
    {
        printf("Messaggio da inviare: ");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (!strncasecmp(buffer, "fine", 4))
        {
            close(sockfd);
            return 0;
        }
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, socklen) < 0)
        {
            perror("ERRORE nell'invio del messaggio al server");
            return errno;
        }
        if((datalen=recvfrom(sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr *)&server_addr,&socklen))<0){
            perror("ERRORE nella ricezione del messaggio del server");
            return errno;
        }
        printf("Messaggio dal server: %s",buffer);
    }

    close(sockfd);

    return 0;
}