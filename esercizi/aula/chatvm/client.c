#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                  \
    do                                              \
    {                                               \
        int __res__ = fun;                          \
        if (__res__ < 0)                            \
        {                                           \
            error_at_line(EXIT_FAILURE, errno,      \
                          __FILE__, __LINE__, "%s", \
                          #fun);                    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    } while (0)
#else
#define debug(fun) fun
#endif

#define MSG_SIZE 21

void receiver_process(struct sockaddr_in *client_addr)
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char msg[MSG_SIZE];

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(bind(sockfd, (struct sockaddr *)client_addr, len));
    debug(listen(sockfd, 10));

    int new_sockfd;
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&server_addr, &len)) > 0)
    {
        if (!fork()) // Figlio
        {
            close(sockfd);
            memset(msg, 0, MSG_SIZE);
            debug(recv(new_sockfd, msg, MSG_SIZE - 1, 0));
            printf("Messaggio ricevuto: '%s'\n", msg);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        close(new_sockfd);
    }
    close(sockfd);
}

void sender_process(struct sockaddr_in *server_addr, const char language[], const char port[])
{
    char input[MSG_SIZE + 4];
    int sockfd;
    char msg[MSG_SIZE];

    memset(msg, 0, MSG_SIZE);
    sprintf(msg, "%s %s", language, port);
    printf("Messaggio di registrazione: %s\nCostruito con language %s e port %s", msg, language, port);

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(connect(sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in)));
    // Registrazione indicando il linguaggio e porta
    debug(send(sockfd, msg, strlen(msg) + 1, 0));

    while (1)
    {
        printf("Inserire la stringa 'n messaggio' > \n");
        fgets(input, MSG_SIZE + 4, stdin);
        input[strcspn(input, "\n")] = '\0';

        send(sockfd, input, strlen(input) + 1, 0);
    }
}

/**
 * @brief Il client rappresenta fra le 5 macchine associate ad un linguaggio di programmazione:
 * C, C++, Java, Python, R, Matlab
 *
 * Il client deve conoscere l'indirizzo e la porta del server, a cui invierà un messaggio dal formato
 * n string
 * con n: numero [10, 255], string di massimo 20 caratteri
 *
 * Tutti i client rimarranno in ascolto su una porta prestabilita per ricevere eventuali comunicazioni dal server
 * come risultato del messaggio di un altro client
 *
 * @param argc numero di argomenti da riga di comando
 * @param argv lista di stringhe passate da riga di comando
 * @return int codice di uscita
 */
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    if (argc < 5 || atoi(argv[2]) == 0 || atoi(argv[4]) == 0)
    {
        fprintf(stderr, "use: %s <linguaggio di programmazione> <porta client> <ip server> <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = client_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(atoi(argv[4]));
    server_addr.sin_addr.s_addr = inet_addr(argv[3]);

    client_addr.sin_port = htons(atoi(argv[2]));
    client_addr.sin_addr.s_addr = INADDR_ANY;

    int pid = fork();

    if (pid == 0)
    {
        receiver_process(&client_addr);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0)
    {
        sender_process(&server_addr, argv[1], argv[2]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return 0;
}