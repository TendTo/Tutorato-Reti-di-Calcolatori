#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define handle(msg)                               \
    do                                            \
    {                                             \
        fprintf(stderr, "[%d] error ", __LINE__); \
        perror(msg);                              \
        exit(EXIT_FAILURE);                       \
    } while (true);

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFSIZ];
    int word_length;
    char c;

    if (argc != 3)
        handle("argc");

    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[2]));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle("socket");
    if ((connect(sockfd, (struct sockaddr *)&server_addr, len)) < 0)
        handle("connect");

    // Register
    printf("Enter username: ");
    fgets(buffer, BUFSIZ, stdin);
    buffer[strcspn(buffer, "\n")] = 0;

    if ((send(sockfd, buffer, BUFSIZ, 0)) < 0)
        handle("send");
    if ((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0)
        handle("recv");
    buffer[n] = 0;
    printf("S: %s\n", buffer);

    if (!strcmp(buffer, "Register failed"))
        exit(EXIT_FAILURE); // Register failed

    if ((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0)
        handle("recv"); // Recv hidden word
    buffer[n] = 0;
    word_length = strlen(buffer);
    printf("\nSecret word's length: %d char\n\n", word_length);

    while (strstr(buffer, "_") && !strstr(buffer, "err: 6"))
    {
        printf("Enter a char: ");
        c = getchar();
        while (getchar() != '\n')
        {
        }

        if ((send(sockfd, &c, 1, 0)) < 0)
            handle("send"); // Send a char
        if ((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0)
            handle("recv"); // Recv outcome
        buffer[n] = 0;
        printf("%s\n", buffer);
    }
    if ((n = recv(sockfd, buffer, BUFSIZ, 0)) < 0)
        handle("recv"); // Recv outcome
    buffer[n] = 0;
    printf("%s\n", buffer);
    close(sockfd);

    return 0;
}