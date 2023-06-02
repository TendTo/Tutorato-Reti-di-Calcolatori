/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Il client chiede la registrazione al servizio fornendo una username, una password e una porta di ascolto di sua scelta.
 * Il server accetta la registrazione dopo aver verificato che la username sia diversa da quelle già fornite da altri client. In caso positivo restituisce un token (un intero a 32 bit), altrimenti restituisce un messaggio di errore al client.
 * Le quadruple “username password porta token” vengono memorizzate dal server su un file.
 * Tutte le successive comunicazioni tra client e server avvengono usando il token, che quindi identifica l’utente.
 * L’utente manda dei messaggi al server, che li inoltra a tutti i client registrati sostituendo al token la username del mittente.
 * Tutti i messaggi usano UDP.
 * Ogni client deve quindi avere la possibilità di inviare un messaggio a tutti gli altri e visualizzare i messaggi spediti dagli altri.
 * Il token deve rimanere segreto tra il server ed il client proprietario del token
 * @version 0.1
 * @date 2023-06-01
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
#include <time.h>

#define MAX_LINE_SIZE 1000
#define MAX_MSG_SIZE 1000
#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20
#define DATABASE_FILE "database.txt"
#define LINE_SEPARATOR " "
#define bool short int
#define true 1
#define false 0

#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                                              \
    do                                                                          \
    {                                                                           \
        int res = fun;                                                          \
        if (res < 0)                                                            \
            error_at_line(EXIT_FAILURE, errno, __FILE__, __LINE__, "%s", #fun); \
    } while (0)
#else
#define debug(fun) fun
#endif

typedef enum
{
    REGISTER = 'r', // Registra un nuovo utente
    SUCCESS = '1',  // L'operazione chiesta al server ha avuto successo
    FAIL = '0',     // L'operazione chiesta al server è fallita
    LOGIN = 'l',    // Prova ad effettuare un login con il token
    SEND = 's',     // Invia un messaggio al server che sarà mandato in broadcast agli altri client
    RECEIVE = 'r',  // Ricevi un messaggio in broadcast dal server
} Operation;

typedef struct
{
    Operation op;
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
    int port;
    int token;
    char message[MAX_MSG_SIZE];
} Message;

/**
 * @brief Cerca un utente nel database controllando se il suo username è presente.
 * Ogni riga del database ha la struttura:
 * <username> <password> <ip> <porta> <token>
 * @param msg messaggio che contiene le informazioni dell'utente
 * @return true se l'utente è stato trovato, false altrimenti
 */
bool search_username(Message *msg)
{
    char line[MAX_LINE_SIZE];
    char *username;
    FILE *database = fopen(DATABASE_FILE, "r");

    while (fgets(line, MAX_LINE_SIZE, database) != NULL)
    {
        username = strtok(line, LINE_SEPARATOR);
        printf("Controllo se l'username '%s' è uguale a '%s'\n", username, msg->username);
        if (strcmp(username, msg->username) == 0)
        {
            fclose(database);
            return true;
        }
    }

    fclose(database);
    return false;
}

/**
 * @brief Registra un nuovo utente nel database.
 * Tale operazione va a buon fine solo se l'username dell'utente non è già presente nel database
 * Verrà aggiunta una nuova riga al file nel formato:
 * <username> <password> <ip> <porta> <token>
 * @param msg messaggio che contiene le informazioni dell'utente
 */
void register_user(Message *msg, char ip[])
{
    // L'username dell'utente era già presente nel file
    if (search_username(msg))
    {
        printf("L'utente e' gia' presente nel database\n");
        msg->op = FAIL;
        return;
    }

    printf("Registrazione dell'utente\n");

    msg->op = SUCCESS;
    msg->token = rand();

    FILE *database = fopen(DATABASE_FILE, "a");
    fprintf(database, "%s %s %s %d %d\n", msg->username, msg->password, ip, msg->port, msg->token);
    fclose(database);
}

/**
 * @brief Ottiene l'username di un utente a partire dal token che viene presentato.
 * In altre parole, scorre le righe del file fino a trovare una corrispondenza con il token, individuando così l'username.
 * Quest'ultimo viene salvato nella struttura msg, al posto dell'username.
 * Ogni riga del file è nel formato:
 * <username> <password> <ip> <porta> <token>
 * @param msg struct che contiene il token e nella quale verrà salvato l'username corrispondente
 * @return true se il token è stato trovato e quindi messo nella struct msg, false altrimenti
 */
bool get_username_by_token(Message *msg)
{
    char line[MAX_LINE_SIZE];
    char *token;
    char *username;
    FILE *database = fopen(DATABASE_FILE, "r");

    while (fgets(line, MAX_LINE_SIZE, database) != NULL)
    {
        username = strtok(line, LINE_SEPARATOR);
        strtok(NULL, LINE_SEPARATOR); // password
        strtok(NULL, LINE_SEPARATOR); // ip
        strtok(NULL, LINE_SEPARATOR); // porta
        token = strtok(NULL, LINE_SEPARATOR);
        if (atoi(token) == msg->token)
        {
            printf("L'utente con token %d e' '%s'\n", msg->token, username);
            strcpy(msg->username, username);
            fclose(database);
            return true;
        }
    }

    printf("Non ho trovato l'utente con token %d\n", msg->token);
    fclose(database);
    return false;
}

/**
 * @brief Invia un messaggio che il server ha ricevuto a tutti gli altri client.
 * @param msg messaggio ricevuto dal client
 */
void send_message(Message *msg, int sockfd)
{
    // L'username non è presente nel database
    if (!get_username_by_token(msg))
    {
        msg->op = FAIL;
        return;
    }

    msg->op = RECEIVE; // Tutti i client riceveranno il messaggio con l'operazione RECEIVE
    msg->token = 0;    // Non voglio che gli altri client ricevano il token segreto

    char line[MAX_LINE_SIZE];
    char *username;
    char *ip;
    char *port;
    FILE *database = fopen(DATABASE_FILE, "r");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    client_addr.sin_family = AF_INET;

    while (fgets(line, MAX_LINE_SIZE, database) != NULL)
    {
        username = strtok(line, LINE_SEPARATOR);
        strtok(NULL, LINE_SEPARATOR);        // password
        ip = strtok(NULL, LINE_SEPARATOR);   // ip
        port = strtok(NULL, LINE_SEPARATOR); // porta
        if (strcmp(username, msg->username)) // Non invio il messaggio dell'utente a sè stesso
        {
            printf("Invio all'utente '%s' con ip '%s' e porta '%s'\n", username, ip, port);

            client_addr.sin_port = htons(atoi(port));
            inet_pton(AF_INET, ip, &client_addr.sin_addr);

            sendto(sockfd, msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
        }
    }

    // Notifico il client originale che l'operazione è stata eseguita correttamente
    msg->op = SUCCESS;
    fclose(database);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_struct_length = sizeof(struct sockaddr_in);
    Message msg;
    char ip[INET_ADDRSTRLEN];

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inizializza il seed di random
    srand(time(NULL));

    // Crea il file database se non esiste
    FILE *database = fopen(DATABASE_FILE, "a");
    fclose(database);

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(&msg, 0, sizeof(msg));

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

    // Preparazione indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Binding dell'indirizzo al socket
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    while (1)
    {
        // Ricezione del messaggio
        debug(recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, &client_struct_length));

        // Decidi che operazione effettuare in base alla richiesta dell'utente
        // Le funzioni modificano la struct Message, che poi sarà usata come risposta da inviare all'utente
        switch (msg.op)
        {
        case REGISTER:
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, client_struct_length);
            register_user(&msg, ip);
            break;
        case SEND:
            send_message(&msg, sockfd);
            break;
        default:
            printf("Operazione sconosciuta: %c\n", msg.op);
            continue;
        }
        sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, client_struct_length);
    }

    close(sockfd);
    return 0;
}