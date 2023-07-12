#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

#define USR_SIZE 20
#define WORD_SIZE 50

bool Login(const char *usr, const char *ipv4, int port)
{
    FILE *fp;
    if (!(fp = fopen("Database.txt", "r")))
        handle("fopen");
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, fp))
    {
        char *usr_r = strtok(line, " ");
        char *ipv4_r = strtok(NULL, " ");
        if (!strcmp(usr, usr_r) && !strcmp(ipv4, ipv4_r))
        {
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

char *Register(const char *usr, const char *ipv4, int port)
{
    if (Login(usr, ipv4, port))
        return "Register failed";

    FILE *fp;
    if (!(fp = fopen("Database.txt", "a+")))
        handle("fopen");

    fprintf(fp, "%s %s %d\n", usr, ipv4, port);
    fclose(fp);
    return "Register successful";
}

char *choose_word(const char *filename)
{
    char word[WORD_SIZE];
    int num_lines = 0;

    FILE *fp;
    if (!(fp = fopen(filename, "r")))
        handle("fopen");

    while (fgets(word, WORD_SIZE, fp))
        num_lines++;

    srand(time(NULL));
    int r = rand() % num_lines;
    rewind(fp);

    for (int i = 0; i < r; i++)
        fgets(word, WORD_SIZE, fp);
    word[strcspn(word, "\n")] = 0;
    fclose(fp);
    return strdup(word);
}

bool handle_word(const char *word, char *hidden_word, char c)
{
    if (!strchr(word, c))
        return false;

    for (int i = 0; i < strlen(word); i++)
    {
        if (word[i] == c)
            hidden_word[i] = c;
    }
    return true;
}

int main(int argc, char *argv[])
{
    int sockfd, newsock, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFSIZ], ipv4[INET_ADDRSTRLEN], usr[USR_SIZE], hidden_word[WORD_SIZE];
    int port;

    if (argc != 2)
        handle("argc");

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[1]));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle("socket");
    if ((bind(sockfd, (struct sockaddr *)&server_addr, len)) < 0)
        handle("bind");
    listen(sockfd, 5);

    char *word = choose_word("Dictionary.txt");
    memset(&hidden_word, '_', strlen(word));
    printf("Secret word = '%s'\n\n", word);

    printf("[+]Server listening...\n\n");
    while (newsock = accept(sockfd, (struct sockaddr *)&client_addr, &len))
    {
        printf("[+]New player connected\n");
        if (!fork())
        {
            close(sockfd);
            if ((n = recv(newsock, buffer, BUFSIZ, 0)) < 0)
                handle("recv"); // Recv a char
            buffer[n] = 0;

            // Usr IP port
            strcpy(usr, buffer);
            inet_ntop(AF_INET, &client_addr.sin_addr, ipv4, INET_ADDRSTRLEN);
            port = ntohs(client_addr.sin_port);
            strcpy(buffer, Register(usr, ipv4, port)); // Register
            printf("\nOutcome register: %s\n", buffer);

            if ((send(newsock, buffer, BUFSIZ, 0)) < 0)
                handle("send"); // Send outcome register
            if ((send(newsock, hidden_word, BUFSIZ, 0)) < 0)
                handle("send"); // Send hidden_word

            int err = 0;
            char c;
            while (strcmp(hidden_word, word) && err < 6)
            {
                if ((n = recv(newsock, &c, 1, 0)) < 0)
                    handle("recv"); // Recv a char
                if (!handle_word(word, hidden_word, c))
                    err++;
                sprintf(buffer, "Word: %s | err: %d\n", hidden_word, err);
                printf("%s\n", buffer);
                if ((send(newsock, buffer, BUFSIZ, 0)) < 0)
                    handle("send"); // Send result
            }

            if (err == 6)
            {
                sprintf(buffer, "Match finish! %s lost, the word was: '%s'\n", usr, word);
                printf("%s\n", buffer);
            }
            else
            {
                sprintf(buffer, "Match finish! %s won, word: '%s'\n", usr, word);
                printf("%s\n", buffer);
            }

            if ((send(newsock, buffer, BUFSIZ, 0)) < 0)
                handle("send");
            free(word);
            exit(EXIT_SUCCESS);
        }
        close(newsock);
    }
    close(sockfd);

    return 0;
}