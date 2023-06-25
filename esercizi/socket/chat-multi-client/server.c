/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare un sistema di chat multiClient - Server con le seguenti caratteristiche:
 * Protocollo TCP
 * Gestione dell’autenticazione con solamente username (scelto autonomamente dall'utente)
 * Login e logout degli utenti (decidere i dettagli)
 * I messaggi spediti da un utente vengono inoltrati dal server a tutti gli utenti connessi in quel momento,
 * anteponendo al messaggio il nome del mittente. Il messaggio non viene restituito a chi lo ha generato.
 * Inviando un messaggio apposito (“who”) il server restituisce l’elenco di tutti gli utenti connessi
 * @version 0.1
 * @date 2023-06-24
 *
 * @copyright Copyright (c) 2023
 */
#include <stdio.h>
#include <stdlib.h>
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

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define DATABASE_FILE "database.txt"
#define TEMP_FILE "database.txt.tmp"
#define MAX_MSG_SIZE 1024
#define MAX_USERNAME_SIZE 64
#define MAX_LINE_SIZE 128
#define LOGIN_SUCCESS ANSI_COLOR_GREEN "Login effettuato con successo" ANSI_COLOR_RESET
#define LOGIN_FAILURE ANSI_COLOR_RED "Login fallito" ANSI_COLOR_RESET
#define LOGOUT_SUCCESS ANSI_COLOR_GREEN "Logout effettuato con successo" ANSI_COLOR_RESET
#define LOGOUT_FAILURE ANSI_COLOR_RED "Logout fallito" ANSI_COLOR_RESET
#define WHO_SUCCESS ANSI_COLOR_GREEN "Elenco utenti connessi:\n" ANSI_COLOR_RESET
#define WHO_FAILURE ANSI_COLOR_RED "Nessun utente connesso" ANSI_COLOR_RESET
#define MSG_SUCCESS ANSI_COLOR_GREEN "Messaggio inviato con successo" ANSI_COLOR_RESET
#define MSG_FAILURE ANSI_COLOR_RED "Messaggio non inviato" ANSI_COLOR_RESET
#define REQUEST_FAILURE ANSI_COLOR_RED "Richiesta non valida" ANSI_COLOR_RESET

typedef enum
{
    LOGIN = 'l',  // Effettua il login
    LOGOUT = 'q', // Effettua il logout
    WHO = 'w',    // Restituisce l'elenco degli utenti connessi
    MSG = 'm',    // Invia un messaggio a tutti gli utenti connessi
} request_type;

typedef struct
{
    request_type request;             // Tipo di richiesta
    char username[MAX_USERNAME_SIZE]; // Username dell'utente che invia la richiesta
    int port;                         // Porta su cui il client che invia la richiesta è in ascolto
    char msg[MAX_MSG_SIZE];           // Messaggio da inviare
} message_struct;
typedef message_struct *message_ptr;
typedef message_struct message_t[1];

/**
 * @brief Effettua il login di un utente
 * Prima di aggiungere l'utente al database, controlla che non sia già presente
 * Le righe del file hanno il seguente formato:
 * <username> <ip> <port>
 * @param msg messaggio contenente l'username dell'utente che vuole effettuare il login
 * @param client_addr indirizzo del client che ha inviato la richiesta
 * @return 1 se il login è andato a buon fine, 0 altrimenti
 */
int login(message_t msg, struct sockaddr_in *client_addr)
{
    char line[MAX_LINE_SIZE];

    // Apriamo il file in modalità read/write
    FILE *db = fopen(DATABASE_FILE, "r+");
    if (db == NULL)
    {
        perror("fopen");
        return 0;
    }

    while (fgets(line, MAX_LINE_SIZE, db))
    {
        char *username = strtok(line, " ");
        // Se l'username è già presente nel file, il login fallisce
        if (strcmp(username, msg->username) == 0)
        {
            fclose(db);
            return 0;
        }
    }

    // Scriviamo sul file l'username e l'indirizzo del client
    fprintf(db, "%s %s %d\n", msg->username, inet_ntoa(client_addr->sin_addr), msg->port);

    // Chiudiamo il file
    debug(fclose(db));

    // Il login è andato a buon fine
    return 1;
}

/**
 * @brief Effettua il logout di un utente
 * L'utente il cui username è contenuto nel messaggio viene rimosso dal database, se presente
 * @param msg messaggio contenente l'username dell'utente che vuole effettuare il logout
 * @return 1 se il logout è andato a buon fine, 0 altrimenti
 */
int logout(message_t msg)
{
    FILE *db = fopen(DATABASE_FILE, "r");
    FILE *tmp = fopen(TEMP_FILE, "w");
    char line[MAX_LINE_SIZE];

    if (db == NULL || tmp == NULL)
    {
        perror("fopen");
        if (db)
            debug(fclose(db));
        if (tmp)
            debug(fclose(tmp));
        return 0;
    }

    while (fgets(line, MAX_LINE_SIZE, db))
    {
        char *username = strtok(line, " ");
        if (strcmp(username, msg->username) != 0)
        {
            line[strlen(line)] = ' '; // Ripristiniamo il carattere spazio
            fputs(line, tmp);
        }
    }

    debug(fclose(db));
    debug(fclose(tmp));
    debug(rename(TEMP_FILE, DATABASE_FILE)); // Rinomina il file temporaneo sostituendo il vecchio
    return 1;
}

/**
 * @brief Produce una lista di tutti gli utenti connessi
 * @param connected_users stringa in cui inserire la lista degli utenti connessi
 */
void who(char connected_users[MAX_MSG_SIZE])
{
    char line[MAX_LINE_SIZE];
    FILE *db = fopen(DATABASE_FILE, "r");
    if (db == NULL)
    {
        perror("fopen");
        return;
    }

    memset(connected_users, 0, MAX_MSG_SIZE); // Inizializziamo la stringa a 0
    strcpy(connected_users, WHO_SUCCESS);     // Inizializziamo la stringa con il messaggio di successo

    while (fgets(line, MAX_LINE_SIZE, db)) // Per ogni riga del file
    {
        char *username = strtok(line, " "); // Prendiamo l'username
        strcat(connected_users, username);  // E lo aggiungiamo alla stringa
        strcat(connected_users, ", ");      // Separando il nome degli utenti con virgola e spazio
    }

    if (strlen(connected_users) == strlen(WHO_SUCCESS)) // Se non ci sono utenti connessi
        strcpy(connected_users, WHO_FAILURE);           // Inizializziamo la stringa con il messaggio di errore
    else
        connected_users[strlen(connected_users) - 2] = '\0'; // Altrimenti rimuoviamo l'ultima virgola e lo spazio

    debug(fclose(db));
}

/**
 * @brief Invia un messaggio a tutti gli utenti connessi
 * @param msg messaggio contenente l'username dell'utente che vuole inviare il messaggio e il messaggio stesso
 * @return 1 se il messaggio è stato inviato correttamente, 0 altrimenti
 */
int send_all(message_t msg)
{
    char line[MAX_LINE_SIZE];
    char msg_to_send[MAX_MSG_SIZE];

    FILE *db = fopen(DATABASE_FILE, "r");
    if (db == NULL)
    {
        perror("fopen");
        return 0;
    }

    while (fgets(line, MAX_LINE_SIZE, db))
    {
        char *username = strtok(line, " "); // Prendiamo l'username
        char *ip = strtok(NULL, " ");       // L'indirizzo IP
        char *port = strtok(NULL, " ");     // E la porta

        if (strcmp(username, msg->username) == 0) // Se il destinatario sarebbe il mittente stesso
            continue;                             // Non gli inviamo il suo stesso messaggio

        int sockfd = socket(AF_INET, SOCK_STREAM, 0); //
        struct sockaddr_in client_addr;

        if (sockfd < 0)
        {
            perror("socket");
            continue;
        }

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = inet_addr(ip);
        client_addr.sin_port = htons(atoi(port));

        if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("Errore nella connessione con l'utente '%s' -> %s:%s\n", username, ip, port);
            debug(close(sockfd));
            continue;
        }

        snprintf(msg_to_send, MAX_MSG_SIZE, "%s: %s", msg->username, msg->msg); // Costruiamo il messaggio da inviare
        debug(send(sockfd, msg_to_send, strlen(msg_to_send) + 1, 0));           // E lo inviamo
        debug(close(sockfd));
    }

    debug(fclose(db));
    return 1;
}

/**
 * @brief Gestisce una richiesta ricevuta dal client
 * @param sockfd socket di comunicazione aperta dal client
 * @param client_addr indirizzo del client
 */
void handle_request(int sockfd, struct sockaddr_in *client_addr)
{
    message_t msg;
    char *response;
    char connected_users[MAX_MSG_SIZE];

    // Riceviamo la richiesta
    while (recv(sockfd, msg, sizeof(msg), 0) > 0)
    {

        printf("Richiesta ricevuta da %s:%d\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
        printf("Operazione richiesta: %c\n", msg->request);

        switch (msg->request)
        {
        case LOGIN:
            response = login(msg, client_addr) ? LOGIN_SUCCESS : LOGIN_FAILURE;
            break;
        case LOGOUT:
            response = logout(msg) ? LOGOUT_SUCCESS : LOGOUT_FAILURE;
            break;
        case WHO:
            who(connected_users);
            response = connected_users;
            break;
        case MSG:
            response = send_all(msg) ? MSG_SUCCESS : MSG_FAILURE;
            break;
        default:
            response = REQUEST_FAILURE;
            break;
        }

        // Inviamo la risposta
        debug(send(sockfd, response, strlen(response) + 1, 0));
    }
}

int main(int argc, char const *argv[])
{
    int sockfd, new_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ci assicuriamo che il file esista, creandolo se necessario
    FILE *fp = fopen(DATABASE_FILE, "a");
    if (fp == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    // Indirizzo utilizzato dal server per fare la bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 10));

    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) > 0)
    {
        int pid = fork();
        if (pid == 0) // child
        {
            close(sockfd);
            handle_request(new_sockfd, &client_addr);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        {
            close(new_sockfd);
            continue;
        }
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
