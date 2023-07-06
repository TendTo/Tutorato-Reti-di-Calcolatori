#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

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

bool login(Message *msg, FILE *fp)
{
    // username password
    rewind(fp);
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, fp))
    {
        char *usr = strtok(line, " ");
        char *pass = strtok(NULL, "\n");
        if (!strcmp(usr, msg->usr) && !strcmp(pass, msg->pass))
            return true;
    }
    return false;
}

char *handle_request(Message *msg)
{
    FILE *fp;
    if (!(fp = fopen("Database.txt", "r")))
        handle("fopen\n");

    if (login(msg, fp))
    {
        system(msg->cmd);
        return "Command executed\n";
    }
    return "[-]Permission denied\n";
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        handle("argc\n");

    int sockfd, newsock, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFSIZ];
    Message msg;

    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[1]));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle("socket\n");
    if ((bind(sockfd, (struct sockaddr *)&server_addr, len)) < 0)
        handle("bind\n");

    listen(sockfd, 5);
    printf("[+]Server listening...\n\n");
    while ((newsock = accept(sockfd, (struct sockaddr *)&server_addr, &len)) > 0)
    {
        if (!fork())
        {
            close(sockfd);
            while (true)
            {
                if ((n = recv(newsock, &msg, sizeof(Message), 0)) < 0)
                    handle("recv\n");
                if (n == 0)
                    exit(EXIT_SUCCESS);
                strcpy(buffer, handle_request(&msg));
                printf("\nOutcome: %s\n", buffer);
                if ((send(newsock, buffer, BUFSIZ, 0)) < 0)
                    handle("recv\n");
            }
        }
        close(newsock);
    }
    close(sockfd);

    return 0;
}