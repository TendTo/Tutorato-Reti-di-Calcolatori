#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>

#define USERNAME_SIZE 20
#define SECRET_SIZE 50
#define LIST_SIZE 128
#define SCREEN "REGISTER = 'r'\nLOGIN = 'l'\nUPDATE = 'u'\nGET_SECRET = 'g'\nEXIT = 'e'\n"

typedef enum {REGISTER = 'r', LOGIN = 'l', UPDATE = 'u', GET_SECRET = 'g', EXIT = 'e'} Operation;

typedef struct 
{
    Operation op;
    char usr[USERNAME_SIZE];
    char usr_secret[USERNAME_SIZE];
    char secret[SECRET_SIZE];
    char list[LIST_SIZE];
    char ip[INET6_ADDRSTRLEN];
}Message;

void handle(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    int sockfd, n;
    struct sockaddr_in6 server_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char buffer[BUFSIZ];
    Message msg;

    if(argc != 3) handle("Error argc\n");

    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, len);

    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);
    server_addr.sin6_port = htons(atoi(argv[2]));

    if((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) handle("Error socket\n");

    printf("%s\n", SCREEN);
    while(true)
    {
        printf("Enter a request: ");
        msg.op = getchar();
        while(getchar() != '\n'){}

        switch (msg.op)
        {
            case REGISTER:
                printf("Username: ");
                fgets(msg.usr, USERNAME_SIZE, stdin);
                msg.usr[strlen(msg.usr)-1] = 0;

                printf("Secret: ");
                fgets(msg.secret, SECRET_SIZE, stdin);
                msg.secret[strlen(msg.secret)-1] = 0;

                printf("List: ");
                fgets(msg.list, LIST_SIZE, stdin);
                msg.list[strlen(msg.list)-1] = 0;
                break;

            case LOGIN:
                printf("Username: ");
                fgets(msg.usr, USERNAME_SIZE, stdin);
                msg.usr[strlen(msg.usr)-1] = 0;
                break;

            case UPDATE:
                printf("Secret: ");
                fgets(msg.secret, SECRET_SIZE, stdin);
                msg.secret[strlen(msg.secret)-1] = 0;

                printf("List: ");
                fgets(msg.list, LIST_SIZE, stdin);
                msg.list[strlen(msg.list)-1] = 0;
                break;

            case GET_SECRET:
                printf("Username: ");
                fgets(msg.usr, USERNAME_SIZE, stdin);
                msg.usr[strlen(msg.usr)-1] = 0;

                printf("Username secret: ");
                fgets(msg.usr_secret, USERNAME_SIZE, stdin);
                msg.usr_secret[strlen(msg.usr_secret)-1] = 0;
                break;
            
            case EXIT:
                close(sockfd);
                exit(EXIT_FAILURE);
                break;

            default:
                printf("Invalid request\n");
                continue;
        }
        if((sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&server_addr, len)) < 0) handle("Error sendto\n");
        if((n = recvfrom(sockfd, buffer, BUFSIZ, 0, (struct sockaddr*)&server_addr, &len)) < 0) handle("Error recvfrom\n");
        buffer[n] = 0;
        printf("\n--> Outcome: %s\n\n", buffer);
    }
    return 0;
}