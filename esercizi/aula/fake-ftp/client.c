#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define LINE_MAX 1024

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char filename[FILENAME_MAX], file_chunk[LINE_MAX];

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);

    server_addr.sin_family = AF_INET;

    // Settiamo l'indirizzo del server
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    printf("Caro utente, che file vuoi prendere dal server?\n");
    fgets(filename, FILENAME_MAX, stdin);
    filename[strlen(filename) - 1] = '\0';
    printf("Voglio il file %s\n", filename);

    // Chiediamo il file al server
    send(sockfd, filename, strlen(filename) + 1, 0);

    FILE *file = fopen(filename, "w");
    int char_read = 0;
    while ((char_read = recv(sockfd, file_chunk, LINE_MAX, 0)) > 0)
        fwrite(file_chunk, sizeof(char), char_read, file);

    fclose(file);

    return 0;
}