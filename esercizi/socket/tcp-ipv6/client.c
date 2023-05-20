#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFERSIZE 1024

void handle_error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sockfd, n, ipv6_port;
    struct sockaddr_in6 dest_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char sendline[BUFFERSIZE];
    char recvline[BUFFERSIZE];
    char ipv6_addr[INET6_ADDRSTRLEN];

    if (argc != 3)
        handle_error("Error argc\n");

    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        handle_error("[-]Error socket\n");
    printf("[+]Socket created succesful\n\n");

    memset(&dest_addr, 0, len);
    dest_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, argv[1], &dest_addr.sin6_addr);
    dest_addr.sin6_port = htons(atoi(argv[2]));

    if ((connect(sockfd, (struct sockaddr *)&dest_addr, len)) < 0)
        handle_error("Error connect\n");
    printf("[+]Client connected succesful\n");

    bzero(sendline, BUFFERSIZE);
    bzero(recvline, BUFFERSIZE);

    for (;;)
    {
        printf("\nEnter a request:\n");
        fgets(sendline, BUFFERSIZE, stdin);
        sendline[strcspn(sendline, "\n")] = 0;

        if (strcmp(sendline, "STOP") == 0)
            break;

        if ((send(sockfd, sendline, BUFFERSIZE, 0)) < 0)
            handle_error("Error send\n");

        if ((n = recv(sockfd, recvline, BUFFERSIZE, 0)) < 0)
            handle_error("Error recv\n");
        recvline[n] = 0;

        inet_ntop(AF_INET6, &(dest_addr.sin6_addr), ipv6_addr, INET6_ADDRSTRLEN);
        ipv6_port = ntohs(dest_addr.sin6_port);
        printf("Reply: %s IP: %s, port: %d\n", recvline, ipv6_addr, ipv6_port);
    }

    close(sockfd);

    return 0;
}