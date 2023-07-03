/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Il server offre un servizio di reverse shell ai client che si connettono ad esso.
 * Prima di poter utilizzare il servizio, il client deve autenticarsi con username e password.
 * Il server controlla che il client autorizzato,
 * verificando sia presente nel file 'database.txt', creato a mano in precedenza,
 * con il formato 'username password'.
 * Se l'autenticazione va a buon fine, il client è libero di eseguire comandi sul server.
 * Il server restituisce al client lo standard output dei comandi eseguiti.
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

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

#define MAX_USERNAME_SIZE 32
#define MAX_PASSWORD_SIZE 32
#define MAX_BUFFER_SIZE 1024
#define DATABASE_FILE "database.txt"

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <server ip> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    // Crea un timeout per la recv
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    debug(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr); // Indirizzo del server
    server_addr.sin_port = htons(atoi(argv[2]));        // Porta del server

    debug(connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len));

    char username[MAX_USERNAME_SIZE], password[MAX_PASSWORD_SIZE];
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0'; // Rimuove il carattere '\n' letto da fgets
    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0'; // Rimuove il carattere '\n' letto da fgets

    // Invio username e password al server
    debug(send(sockfd, username, sizeof(username), 0));
    debug(send(sockfd, password, sizeof(password), 0));

    // Ricezione della risposta del server
    char response[MAX_BUFFER_SIZE + 1];
    debug(recv(sockfd, response, sizeof(response), 0));
    printf("Risultato dell'operazione: %s\n", response);

    if (strcmp(response, "Autenticazione fallita") == 0)
    {
        debug(close(sockfd));
        exit(EXIT_FAILURE);
    }

    // Se l'autenticazione è andata a buon fine, il client può eseguire comandi sul server
    char input[MAX_BUFFER_SIZE + 1];
    while(1)
    {
        printf("Inserisci un comando: ");
        fgets(input, sizeof(input), stdin);
        debug(send(sockfd, input, strlen(input), 0));

        // Ricezione della risposta del server
        char response[MAX_BUFFER_SIZE + 1];
        memset(response, '\0', sizeof(response));
        recv(sockfd, response, sizeof(response), 0); // Dopo 3 secondi va in timeout
        printf("%s", response);
    }

    debug(close(sockfd));
    return 0;
}
