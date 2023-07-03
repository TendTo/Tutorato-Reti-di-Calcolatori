/**
 * @file server.c
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
#define DATABASE_FILE "database.txt"

/**
 * @brief Verifica che l'utente sia presente nel database.
 * Le credenziali sono nel formato 'username password'.
 * Se l'utente è presente, restituisce 1, altrimenti 0.
 * @param username username dell'utente
 * @param password password dell'utente
 * @return 1 se l'utente è presente nel database, 0 altrimenti
 */
int login(const char username[], const char password[])
{
    FILE *database = fopen(DATABASE_FILE, "r");
    if (database == NULL)
        return 0;

    char line[MAX_USERNAME_SIZE + MAX_PASSWORD_SIZE + 2]; // +2 per '\n' e '\0'
    char *db_username, *db_password;
    while (fgets(line, sizeof(line), database) != NULL)
    {
        db_username = strtok(line, " ");
        db_password = strtok(NULL, "\n");

        if (strcmp(username, db_username) == 0 && strcmp(password, db_password) == 0)
        {
            printf("Utente trovato\n");
            fclose(database);
            return 1;
        }
    }

    fclose(database);
    return 0;
}

/**
 * @brief Apre una reverse shell sul socket fornito.
 * Il client sarà in grado di eseguire comandi sul server.
 * Viene utilizzata la shell bash.
 * @param sockfd socket a cui il client è connesso
 */
void reverse_shell(int sockfd)
{
    // Duplicazione del socket su stdin, stdout e stderr per il processo che avvierà la shell
    debug(dup2(sockfd, STDIN_FILENO));
    debug(dup2(sockfd, STDOUT_FILENO));
    debug(dup2(sockfd, STDERR_FILENO));

    // Apertura della shell
    execl("/bin/bash", "/bin/bash", NULL);
}

/**
 * @brief Gestisce la richiesta del client.
 * @param sockfd socket a cui il client è connesso
 */
void handle_request(int sockfd)
{
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    // Lettura username
    debug(read(sockfd, username, sizeof(username)));
    // Lettura password
    debug(read(sockfd, password, sizeof(password)));

    // Autenticazione
    if (login(username, password) == 0)
    {
        // Autenticazione fallita
        printf("Autenticazione fallita: '%s' '%s' non trovato\n", username, password);
        debug(send(sockfd, "Autenticazione fallita", sizeof("Autenticazione fallita"), 0));
        return;
    }

    // Autenticazione riuscita
    printf("Autenticazione riuscita: '%s' '%s' trovato\n", username, password);
    debug(send(sockfd, "Autenticazione riuscita", sizeof("Autenticazione riuscita"), 0));

    // Reverse shell
    reverse_shell(sockfd);
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 5));

    // Accettazione connessione
    int new_sockfd;
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        // Creazione processo figlio
        if (fork() == 0)
        {
            // Chiusura del socket di ascolto
            debug(close(sockfd));

            // Autenticazione
            handle_request(new_sockfd);

            // Chiusura del socket
            debug(close(new_sockfd));

            // Terminazione del processo figlio
            exit(EXIT_SUCCESS);
        }
        debug(close(new_sockfd));
    }

    return 0;
}
