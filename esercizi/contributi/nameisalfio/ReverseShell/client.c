#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define USR_SIZE 20
#define PASS_SIZE 20
#define COMMAND_SIZE 50

typedef struct
{
    char usr[USR_SIZE];
    char pass[PASS_SIZE];
    char cmd[COMMAND_SIZE];
} Message;

void handle(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        handle("argc\n");

    int sockfd, n;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFSIZ];
    Message msg;

    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, len);

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[2]));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle("socket\n");
    if ((connect(sockfd, (struct sockaddr *)&server_addr, len)) < 0)
        handle("connect\n");

    while (true)
    {
        printf("Enter a command: ");
        fgets(msg.cmd, COMMAND_SIZE, stdin);
        msg.cmd[strcspn(msg.cmd, "\n")] = 0;

        if (!strcmp(msg.cmd, "exit"))
            break;

        printf("Enter username: ");
        fgets(msg.usr, USR_SIZE, stdin);
        msg.usr[strcspn(msg.usr, "\n")] = 0;

        printf("Enter password: ");
        fgets(msg.pass, PASS_SIZE, stdin);
        msg.pass[strcspn(msg.pass, "\n")] = 0;

        if ((send(sockfd, &msg, sizeof(Message), 0)) < 0)
            handle("send\n");
        if ((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0)
            handle("recv\n");
        buffer[n] = 0;
        printf("Server: %s\n", buffer);
    }
    return 0;
}

/*
    int fd, n_read;
    while(true)
    {
        printf("Enter a request: ");
        fgets(buffer, BUFSIZ, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if((send(sockfd, buffer, strlen(buffer), 0)) < 0) handle("send\n");

        printf("Enter a filename: ");
        fgets(buffer, BUFSIZ, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        //if(!strcasecmp(buffer, "EXIT")) break;

        if((fd = open(buffer, O_RDONLY)) < 0) handle("open\n");
        while((n_read = read(fd, buffer, BUFSIZ)) > 0)
            write(sockfd, buffer, n_read);

        if((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0) handle("recv\n");
        buffer[n] = 0;
        printf("Server: %s\n\n", buffer);
    }
    close(sockfd);
*/