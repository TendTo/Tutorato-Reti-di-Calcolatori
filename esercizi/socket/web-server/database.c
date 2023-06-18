/**
 * @file database.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Web server che si occupa di gestire le richieste di connessione da un client servendo una pagina html.
 * Nel momento in cui riceve una richiesta, il server crea un processo figlio che si occuperà di richiedere la pagina html al database.
 * Il database è un server che si occupa di gestire le richieste di connessione da parte del server web.
 * Cerca di trovare il path richiesto dell'url del client e, se lo trova, restituisce la pagina html corrispondente.
 * @version 0.1
 * @date 2023-06-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

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

#define N_ACCEPT 10
#define MAX_REQ_SIZE 1024
#define MAX_HTML_SIZE 4096
#define DATABASE_PORT 3306

/**
 * @brief Controlla se la stringa str termina con la stringa suffix.
 * @param str stringa da controllare
 * @param suffix suffisso da cercare
 * @return 0 se str non termina con suffix, 1 altrimenti
 */
int str_ends_with(const char str[], const char suffix[])
{
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);
    return str_len >= suffix_len && strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

/**
 * @brief Restituisce il contenuto del file html corrispondente al path url.
 * @param url url richiesto dal client
 * @param html contenuto del file da restituire
 */
void get_html_file(char url[], char html[])
{
    char *path = url;
    FILE *fp;

    if (strcmp(path, "/") == 0)
        path = "index.html";
    while (*path == '/')
        path++;
    printf("Searching for %s\n", path);
    if (!str_ends_with(path, ".html") && !str_ends_with(path, ".css") && !str_ends_with(path, ".js"))
        return;

    fp = fopen(path, "r");
    if (fp == NULL)
        return;
    fread(html, sizeof(char), MAX_HTML_SIZE, fp);
    fclose(fp);
}

void child_process(int sockfd_conn)
{
    char req[MAX_REQ_SIZE], html[MAX_HTML_SIZE];

    memset(req, '\0', sizeof(req));
    memset(html, '\0', sizeof(html));

    debug(recv(sockfd_conn, req, sizeof(req), 0));
    printf("New request from server: %s\n", req);

    get_html_file(req, html);

    // Inviamo la risposta al client
    debug(send(sockfd_conn, html, strlen(html), 0));

    printf("Html sent\n");
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, database_addr;
    socklen_t server_len = sizeof(server_addr);

    if (argc < 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&database_addr, 0, sizeof(database_addr));

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));

    // Si impostano le informazioni del database
    database_addr.sin_family = AF_INET;
    database_addr.sin_port = htons(DATABASE_PORT);
    database_addr.sin_addr.s_addr = INADDR_ANY;

    // Abilita il riutilizzo dell'indirizzo
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));
    // Per poter fare la bind sulla porta 80 è necessario avere i privilegi di root
    debug(bind(sockfd, (struct sockaddr *)&database_addr, sizeof(database_addr)));
    debug(listen(sockfd, N_ACCEPT));

    char listen_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(database_addr.sin_addr), listen_ip, INET_ADDRSTRLEN);
    printf("Serving database on %s port %d (http://%s:%d/)\n", listen_ip, DATABASE_PORT, listen_ip, DATABASE_PORT);

    while (1)
    {
        int sockfd_conn;
        debug(sockfd_conn = accept(sockfd, (struct sockaddr *)&server_addr, &server_len));

        int pid = fork();
        if (pid == -1)
        {
            perror("Error while forking");
            close(sockfd_conn);
            continue;
        }
        else if (pid == 0)
        {
            close(sockfd);
            child_process(sockfd_conn);
            close(sockfd_conn);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(sockfd_conn);
            continue;
        }
    }

    // Si chiude la socket
    close(sockfd);

    return 0;
}
