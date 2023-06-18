/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Web server che si occupa di gestire le richieste di connessione da un client servendo una pagina html.
 * Nel momento in cui riceve una richiesta, il server crea un processo figlio che si occuperà di richiedere la pagina html al database.
 * Questa verrà poi elaborata, inserendo l'ip del richiedente e il timestamp ogni volta che trova rispettivamente {{ip}} o {{date}}, e inviata al client.
 * Per poter lanciare il programma sulla porta 80 è necessario eseguirlo con i privilegi di root.
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
#define MAX_HEADER_SIZE 128
#define MAX_RES_SIZE 4096 + MAX_HEADER_SIZE
#define MAX_HTML_SIZE 4096
#define DATE_SIZE 30
#define HTTP_PORT 80
#define DATABASE_PORT 3306
#define IP_TEMPLATE "{{ip}}"
#define DATE_TEMPLATE "{{date}}"

/**
 * @brief Apre una connessione con il database.
 * Richiede un url al database e attende che quest'ultimo restituisca la pagina html corrispondente.
 * @param database_addr indirizzo del database
 * @param url url richiesto dal client
 * @param html pagina html restituita dal database
 * @return 200 se la connessione è avvenuta con successo, 404 se l'url non è presente nel database, 500 altrimenti
 */
int get_html_from_db(const struct sockaddr_in *database_addr, const char url[], char html[])
{
    int sockfd, html_len = 0, res;
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    if (connect(sockfd, (struct sockaddr *)database_addr, sizeof(*database_addr)) < 0)
        return 500;
    if (send(sockfd, url, strlen(url), 0) < 0)
        return 500;
    while (html_len < MAX_HTML_SIZE && (res = recv(sockfd, html + html_len, MAX_HTML_SIZE, 0)) > 0)
        html_len = strlen(html);
    if (res < 0)
        return 500;
    close(sockfd);

    if (strlen(html) == 0)
        return 404;
    else
        return 200;
}

/**
 * @brief Costruisce la risposta http da inviare al client.
 * @param code codice di stato http
 * @param headers header opzionali da inserire nella risposta. Ogni riga deve terminare con "\r\n". Può essere NULL.
 * @param content contenuto della risposta. Può essere html o un messaggio di errore. Può essere NULL.
 * @param res buffer in cui inserire la risposta http
 */
void build_http_response(int code, const char headers[], const char content[], char res[])
{
    char *status;
    switch (code)
    {
    case 200:
        status = "OK";
        break;
    case 400:
        status = "Bad Request";
        break;
    case 404:
        status = "Not Found";
        break;
    default:
        status = "Internal Server Error";
        break;
    }
    sprintf(res, "HTTP/1.1 %d %s\r\n%s\r\n%s", code, status, headers ? headers : "", content ? content : "");
}

/**
 * @brief Ottiene la data corrente e la inserisce in una stringa nel formato:
 * "DD/MMM/YYYY HH:MM:SS"
 * @param date_string stringa in cui inserire la data formattata
 */
void get_date_string(char date_string[])
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date_string, DATE_SIZE, "%d/%b/%Y %H:%M:%S", t);
}

/**
 * @brief Ottiene l'url dalla richiesta http.
 * La richiesta ha il seguente formato:
 * GET /url HTTP/1.1
 * @param request ricevuta dal client
 * @param url url estratto dalla richiesta
 * @return 1 se l'url è stato estratto correttamente, 0 altrimenti
 */
int get_url_from_request(const char request[], char url[])
{
    char *start = strstr(request, "GET") + 4;
    char *end = strstr(start, "HTTP/1.1") - 1;
    if (start == NULL || end == NULL)
        return 0;
    strncpy(url, start, end - start);
    return 1;
}

/**
 * @brief Sostituisce le occorrenze di IP_TEMPLATE e DATE_TEMPLATE con l'ip del client e la data corrente nella pagina html.
 * @param html pagina html da inviare al client
 * @param client_ip ip del client
 * @param date data corrente
 */
void template_html(char html[], const char client_ip[], const char date[])
{
    char tmp[MAX_HTML_SIZE], *ptr, *template_pos;
    char *templates[] = {IP_TEMPLATE, DATE_TEMPLATE};
    for (int i = 0; i < sizeof(templates) / sizeof(char *); i++)
    {
        while (template_pos = strstr(html, templates[i]))
        {
            memset(tmp, '\0', MAX_HTML_SIZE);
            ptr = html;
            strncat(tmp, ptr, template_pos - ptr);
            if (strcmp(templates[i], IP_TEMPLATE) == 0)
                strncat(tmp, client_ip, strlen(client_ip));
            else if (strcmp(templates[i], DATE_TEMPLATE) == 0)
                strncat(tmp, date, strlen(date));
            ptr = template_pos + strlen(templates[i]);
            strncat(tmp, ptr, strlen(ptr));
            strncpy(html, tmp, MAX_HTML_SIZE);
        }
    }
}

void child_process(int sockfd_conn, const struct sockaddr_in *client_addr, const struct sockaddr_in *database_addr)
{
    char req[MAX_REQ_SIZE], date[DATE_SIZE], client_ip[INET_ADDRSTRLEN], res[MAX_RES_SIZE], url[MAX_REQ_SIZE], html[MAX_HTML_SIZE];
    memset(url, '\0', sizeof(url));
    memset(date, '\0', sizeof(date));
    memset(client_ip, '\0', sizeof(client_ip));
    memset(req, '\0', sizeof(req));
    memset(res, '\0', sizeof(res));
    memset(html, '\0', sizeof(html));

    // Formattazione della data
    get_date_string(date);
    // Si ottiene l'ip del client
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);

    printf("%s - [%s] \"New HTTP connection\"\n", client_ip, date);

    debug(recv(sockfd_conn, req, sizeof(req), 0));
    if (!get_url_from_request(req, url))
    {
        build_http_response(400, "Content-Type: text/plain\r\n", "400 Bad request", res);
        debug(send(sockfd_conn, res, strlen(res), 0));
        return;
    }

    printf("%s - [%s] \"GET %s HTTP/1.1\"\n", client_ip, date, url);

    // Otteniamo l'html dal database
    int code = get_html_from_db(database_addr, url, html);
    template_html(html, client_ip, date);

    // Costruiamo la risposta http
    build_http_response(code, "Content-Type: text/html\r\n", html, res);

    // Inviamo la risposta al client
    debug(send(sockfd_conn, res, strlen(res), 0));

    printf("%s - [%s] \"Response sent\" %d\n", client_ip, date, code);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr, database_addr;
    socklen_t client_struct_length = sizeof(client_addr);

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <database ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));

    // Si impostano le informazioni del server. La porta è quella di default per HTTP, 80
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HTTP_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Si impostano le informazioni del database
    database_addr.sin_family = AF_INET;
    database_addr.sin_port = htons(DATABASE_PORT);
    debug(inet_pton(AF_INET, argv[1], &(database_addr.sin_addr)));

    // Abilita il riutilizzo dell'indirizzo
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));
    // Per poter fare la bind sulla porta 80 è necessario avere i privilegi di root
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, N_ACCEPT));

    char listen_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), listen_ip, INET_ADDRSTRLEN);
    printf("Serving HTTP on %s port %d (http://%s:%d/)\n", listen_ip, HTTP_PORT, listen_ip, HTTP_PORT);

    while (1)
    {
        int sockfd_conn;
        debug(sockfd_conn = accept(sockfd, (struct sockaddr *)&client_addr, &client_struct_length));

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
            child_process(sockfd_conn, &client_addr, &database_addr);
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
