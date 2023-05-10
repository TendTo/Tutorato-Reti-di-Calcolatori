/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Server che si occupa dell'autenticazione.
 * Verrà creato un file di testo 'database.txt' che conterrà le coppie username,password.
 * Gli utenti si possono registrare al server, a patto che il loro username sia univoco.
 * Utilizza ipv6.
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define SEPARATOR ","
#define MAX_MSG_SIZE 1000
#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20
#define MAX_LINE_SIZE MAX_PASSWORD_SIZE + 1 + MAX_USERNAME_SIZE
#define SUCCESS_MESSAGE "Aggiunta effettuata con successo!"
#define ALREADY_PRESENT_MESSAGE "L'utente è già registrato"
#define ILLEGAL_CHAR_MESSAGE "Il simbolo '" SEPARATOR "' non può essere usato all'intero di nome utente o password"

typedef struct
{
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
} User;

bool search_user(const char username[], FILE *file)
{
    char riga[MAX_LINE_SIZE];
    bool found = false;

    rewind(file);
    while (fgets(riga, sizeof(riga), file) != NULL)
    {
        // Taglia la riga al primo separatore
        riga[strcspn(riga, SEPARATOR)] = '\0';
        if (strcmp(username, riga) == 0)
        {
            found = true;
            break;
        }
    }

    return found;
}

bool login(User u, FILE *f)
{
    // Controlla se l'utente è già presente
    if (search_user(u.username, f))
        return false;

    // L'utente viene registrato alla fine del file con un formato "username,password"
    fseek(f, 0, SEEK_END);
    fprintf(f, "%s" SEPARATOR "%s\n", u.username, u.password);
    fflush(f);
    return true;
}

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t sock_len = sizeof(struct sockaddr_in6);
    socklen_t client_sock_len = sock_len;
    char msg[MAX_MSG_SIZE];

    if (argc < 2)
    {
        printf("Use: listening_PORT");
        return 0;
    }

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error in socket opening ... exit!");
        return 1;
    }

    memset(&client_addr, 0, sock_len);
    memset(&server_addr, 0, sock_len);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[1]));
    server_addr.sin6_addr = in6addr_any;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sock_len) < 0)
    {
        perror("Binding error!");
        return 1;
    }

    FILE *f = fopen("database.txt", "r+");
    if (f == NULL)
    {
        perror("Errore nell'apertura del file");
        return 1;
    }

    while (1)
    {
        if ((n = recvfrom(sockfd, msg, MAX_MSG_SIZE, 0, (struct sockaddr *)&client_addr, &client_sock_len)) > 0)
        {
            User u;
            memcpy(&u, msg, sizeof(User));

            char *server_response;
            if (strstr(u.username, SEPARATOR) != NULL || strstr(u.password, SEPARATOR) != NULL)
                server_response = ILLEGAL_CHAR_MESSAGE;
            else if (login(u, f))
                server_response = SUCCESS_MESSAGE;
            else
                server_response = ALREADY_PRESENT_MESSAGE;

            if (sendto(sockfd, server_response, strlen(server_response) + 1, 0, (struct sockaddr *)&client_addr, client_sock_len) < 0)
                perror("Errore nella risposta al client");
        }
    }

    fclose(f);
    close(sockfd);
    return 0;
}
