/**
 * @file client.c
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
#include <unistd.h> // for close()

#define MAX_LINE_SIZE 1000
#define MAX_MSG_SIZE 1000
#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20
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
    EXIT = 'e',     // Esci dall'applicazione
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
 * @brief Inizializza la struttura msg con nome utente e password selezionate dall'utente da riga di comando.
 * Da questo momento in poi la struttura potrà essere inviata al server per effettuare la registrazione
 * @param msg struttura del messaggio da inviare al server
 */
void setup_register(Message *msg)
{
    printf("Inserisci username: ");
    fgets(msg->username, sizeof(msg->username), stdin);
    msg->username[strcspn(msg->username, "\n")] = '\0';

    printf("Inserisci password: ");
    fgets(msg->password, sizeof(msg->password), stdin);
    msg->password[strcspn(msg->password, "\n")] = '\0';
}

/**
 * @brief Permette all'utente di inserire un messaggio da terminale.
 * @param msg
 */
void setup_send(Message *msg)
{
    printf("Inserisci il messaggio: ");
    fgets(msg->message, sizeof(msg->message), stdin);
    msg->message[strcspn(msg->message, "\n")] = '\0';
}

/**
 * @brief Tutti i client hanno un processo che si limita a fare da server ed ascoltare le richieste in arrivo dal server.
 * In questo modo possono ricevere i messaggi in broadcast che altri client inviano loro.
 * Di base, si comportano a tutti gli effetti come un server.
 */
void client_receiver(int sockfd)
{
    Message msg;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);

    printf("Il client si e' messo in ascolto per comunicazioni broadcast ricevute dal server\n");
    while (1)
    {
        recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (msg.op == RECEIVE)
            printf("Ho ricevuto il seguente messaggio dal server.\nMittente: %s\nMessaggio: %s\n", msg.username, msg.message);
        else
            printf("Ho ricevuto una comunicazione che non sono sicuro come interpretare\n");
    }
}

/**
 * @brief La parte interattiva del client.
 * Fornisce all'utente dei prompt per stabilire che operazioni effettuare:
 * - r: registrarsi con il server
 * - s: inviare un nuovo messaggio in broadcast
 * - e: terminare il client
 */
void client_sender(int port, struct sockaddr *server_addr)
{
    int sockfd, token = 0;
    Message msg;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);

    memset(&msg, 0, sizeof(Message));

    // Questa socket si occuperà della comunicazione con il server.
    // Non è necessario, perciò, fare una bind
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

    while (1)
    {
        printf("Inserisci operazione da effettuare:\nr: registrazione\ns: send\ne: exit\n");
        msg.op = getchar();
        getchar();

        switch (msg.op)
        {
        case REGISTER:
            setup_register(&msg);
            msg.port = port;

            printf("Utente %s, password %s\n", msg.username, msg.password);

            sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)server_addr, server_addr_len);
            recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)server_addr, &server_addr_len);

            token = msg.token;
            printf("Utente %s, password %s e ho ricevuto questo token dal server: %d\n", msg.username, msg.password, msg.token);
            break;
        case SEND:
            if (token == 0)
            {
                fprintf(stderr, "Non hai ancora ricevuto un token dal server! Registrati!\n");
                continue;
            }
            msg.token = token;
            setup_send(&msg);
            sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)server_addr, server_addr_len);
            recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)server_addr, &server_addr_len);
            printf("Il server ha risposto: l'operazione %s\n", msg.op == SUCCESS ? "ha avuto successo" : "e' fallita");
            break;
        case EXIT:
            close(sockfd);
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_struct_length = sizeof(struct sockaddr_in);
    char ip[INET_ADDRSTRLEN];

    if (argc < 4 || atoi(argv[1]) == 0 || atoi(argv[3]) == 0)
    {
        fprintf(stderr, "Usage: %s <client port> <server ip> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));

    // Preparazione indirizzo del client
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, argv[2], &client_addr.sin_addr); // Accetto connessioni solo dal server

    // Preparazione indirizzo del server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &server_addr.sin_addr);

    // Binding dell'indirizzo al socket
    debug(bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)));

    int pid = fork();

    // TODO: fork per gestire sia la parte di ricezione che quella d'invio
    if (pid == -1)
    {
        perror("Errore nella fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // Processo figlio
    {
        client_receiver(sockfd);
        close(sockfd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(sockfd);
        client_sender(atoi(argv[1]), (struct sockaddr *)&server_addr);
        exit(EXIT_SUCCESS);
    }

    return 0;
}