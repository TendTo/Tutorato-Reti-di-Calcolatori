/**
 * @file client.c
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

#define MAX_MSG_SIZE 1024
#define MAX_USERNAME_SIZE 64
#define MISSING_USERNAME "Username non specificato"
#define REQUEST_FAILURE "Richiesta non valida"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef enum
{
    LOGIN = 'l',  // Effettua il login
    LOGOUT = 'q', // Effettua il logout
    WHO = 'w',    // Restituisce l'elenco degli utenti connessi
    MSG = 'm',    // Invia un messaggio a tutti gli utenti connessi
    EXIT = 'e'    // Termina il programma
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
 * @brief Il processo figlio si mette in ascolto su una porta prestabilita
 * per ricevere eventuali comunicazioni dal server come risultato del messaggio di un altro client
 * @param client_addr sockaddr su cui mettersi in ascolto
 */
void receiver_process(struct sockaddr_in *client_addr)
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char msg[MAX_MSG_SIZE];

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));
    debug(bind(sockfd, (struct sockaddr *)client_addr, len));
    debug(listen(sockfd, 10));

    int new_sockfd;
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&server_addr, &len)) > 0)
    {
        if (!fork()) // Figlio
        {
            close(sockfd);
            memset(msg, 0, MAX_MSG_SIZE);
            debug(recv(new_sockfd, msg, MAX_MSG_SIZE - 1, 0));
            printf(ANSI_COLOR_BLUE "\n%s\n" ANSI_COLOR_RESET, msg);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        close(new_sockfd);
    }
    close(sockfd);
}

/**
 * @brief Non appena avviato, il processo padre si registra con il server, inviando la coppia
 * <linguaggio di programmazione> <porta su cui è in ascolto>.
 *
 * Successivamente, si mette in attesa di input da tastiera
 * per poi inviare al server messaggi nel formato
 * <n> <string>
 * @param server_addr sockaddr del server
 * @param language linguaggio di programmazione del client
 * @param port porta su cui il client è in ascolto
 */
void sender_process(struct sockaddr_in *server_addr, int port)
{
    int sockfd;
    message_t msg;
    char response[MAX_MSG_SIZE];

    memset(msg, 0, sizeof(message_t));
    msg->port = port;

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(connect(sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in)));

    while (1)
    {
        printf("Operazione da eseguire:\n");
        printf("[l] Login\n");
        printf("[q] Logout\n");
        printf("[w] Who\n");
        printf("[m] Messaggio\n");
        printf("[e] Esci\n");
        printf("Scelta: ");
        msg->request = getchar(); // Considera solo il primo carattere inserito
        while (getchar() != '\n') // Svuota il buffer fino ad una nuova linea
            ;

        switch (msg->request)
        {
        case LOGIN:
            printf("Username: ");
            fgets(msg->username, MAX_USERNAME_SIZE, stdin);
            msg->username[strlen(msg->username) - 1] = '\0'; // Sostituisce il carattere di newline con il terminatore di stringa
            break;
        case MSG:
            printf("Messaggio: ");
            fgets(msg->msg, MAX_MSG_SIZE, stdin);
            msg->msg[strlen(msg->msg) - 1] = '\0'; // Sostituisce il carattere di newline con il terminatore di stringa
            // Fall through
        case LOGOUT:
        case WHO:
            if (strlen(msg->username) == 0) // Controlla che l'utente abbia almeno un username impostato
            {
                fprintf(stderr, MISSING_USERNAME "\n");
                continue;
            }
            break;
        case EXIT:
            close(sockfd);
            return;
        default:
            fprintf(stderr, REQUEST_FAILURE "\n");
            continue;
        }

        memset(response, 0, MAX_MSG_SIZE);
        debug(send(sockfd, msg, sizeof(message_t), 0));
        debug(recv(sockfd, response, MAX_MSG_SIZE, 0));
        printf("%s\n", response);
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

    if (argc < 4 || atoi(argv[1]) == 0 || atoi(argv[3]) == 0)
    {
        fprintf(stderr, "use: %s <porta client> <ip server> <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = client_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(atoi(argv[3]));
    server_addr.sin_addr.s_addr = inet_addr(argv[2]);

    client_addr.sin_port = htons(atoi(argv[1]));
    client_addr.sin_addr.s_addr = INADDR_ANY;

    int pid = fork();

    if (pid == 0)
    {
        receiver_process(&client_addr);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0)
    {
        sender_process(&server_addr, atoi(argv[1]));
        exit(EXIT_SUCCESS);
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return 0;
}