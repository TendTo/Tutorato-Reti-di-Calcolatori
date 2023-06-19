#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define LINE_MAX 1024

void send_file(int sockfd)
{
    char filename[FILENAME_MAX], file_chunk[LINE_MAX];

    recv(sockfd, filename, FILENAME_MAX, 0);
    printf("Il client vuole il file %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Opening file");
        return;
    }

    int char_read = 0;
    while ((char_read = fread(file_chunk, sizeof(char), LINE_MAX, file)) > 0)
        send(sockfd, file_chunk, char_read, 0);
}

int main(int argc, char *argv[])
{
    int sockfd, new_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char filename[FILENAME_MAX], file_line[LINE_MAX];

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    // Settiamo l'indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("connect()");
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("listen()");
    }

    while (new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len))
    {
        int pid = fork();

        if (pid == 0)
        {
            close(sockfd);
            send_file(new_sockfd);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }

        close(new_sockfd);
    }

    return 0;
}