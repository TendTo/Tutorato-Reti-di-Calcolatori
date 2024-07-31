#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    int sockfd, new_sock, n, ipv6_port;
    struct sockaddr_in6 local_addr, remote_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char sendline[BUFFERSIZE];
    char recvline[BUFFERSIZE];
    char ipv6_addr[INET6_ADDRSTRLEN];

    if (argc != 2)
        handle_error("Error argc\n");

    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        handle_error("[-]Error socket\n");
    printf("[+]Socket created succesful\n\n");

    memset(&local_addr, 0, len);
    local_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::", &local_addr.sin6_addr);
    local_addr.sin6_port = htons(atoi(argv[1]));
    // Se si utilizzano ip link local (che iniziano con fe80::) bisogna specificare l'interfaccia
    // server_addr.sin6_scope_id = if_nametoindex("enp0s3"); // dove enp0s3 è il nome dell'interfaccia

    if ((bind(sockfd, (struct sockaddr *)&local_addr, len)) < 0)
        handle_error("Error bind\n");

    listen(sockfd, 5);
    printf("Server listening...\n\n");

    bzero(sendline, BUFFERSIZE);
    bzero(recvline, BUFFERSIZE);

    for (;;)
    {
        new_sock = accept(sockfd, (struct sockaddr *)&remote_addr, &len);
        inet_ntop(AF_INET6, &(remote_addr.sin6_addr), ipv6_addr, INET6_ADDRSTRLEN);
        ipv6_port = ntohs(remote_addr.sin6_port);
        printf("Client connected    IP: %s, port: %d\n", ipv6_addr, ipv6_port);

        if (!fork())
        {
            close(sockfd);
            for (;;)
            {
                if ((n = recv(new_sock, recvline, BUFFERSIZE, 0)) < 0)
                    handle_error("Error recv\n");
                recvline[n] = 0;

                printf("\nMessage: %s IP: %s, port: %d\n", recvline, ipv6_addr, ipv6_port);

                sprintf(sendline, "%s", "ACK");
                if ((send(new_sock, sendline, BUFFERSIZE, 0)) < 0)
                    handle_error("Error send\n");
            }
            close(new_sock);
        }
        else
            close(new_sock);
    }
    close(sockfd);

    return 0;
}