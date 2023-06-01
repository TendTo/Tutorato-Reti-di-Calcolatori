/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare il seguente servizio di trasferimento file. La rete è formata da un server e da vari client, tutti uguali.
 * Il client si registra sul server, fornendo una username, una lista di file disponibili per la copia ed una porta di ascolto (TCP, vedi oltre)
 * Una volta registrato, un client può chiedere al server la lista degli altri utenti registrati e la lista dei file condivisi da ciascun client (identificato con username).
 * Quando vuole scaricare un file, il client chiede al server le informazioni associate allo username con cui vuole connettersi, ovvero l’indirizzo IP e la porta TCP di ascolto (il trasferimento avviene su un canale TCP)
 * Il download avviene connettendosi direttamente col client, fornendo il nome del file da scaricare.
 * Periodicamente il server interroga i client, per:
 * - verificare che sono ancora attivi
 * - aggiornare la lista dei file condivisi
 * Tutti i messaggi tra client e server sono UDP
 * @version 0.1
 * @date 2023-06-01
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 1024
#define MAX_USR_SIZE 128

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
    REGISTER = 'r',      // registra un nuovo utente
    LIST_USERS = 'u',    // Elenca gli utenti registrati al server
    LIST_FILES = 'f',    // Elenca i file condivisi da un utente
    INFO_USER = 'i',     // Ottieni le informazioni di un utente (ip e porta)
    DOWNLOAD_FILE = 'd', // Scarica il file da un altro utente
    PING = 'p',          // Il server pinga il client per assicurarsi sia ancora online
    UPDATE_FILES = 'l',  // Il server chiede al client se la lista di file è cambiata
    EXIT = 'e',          // Chiudi il client
} Operation;

typedef struct
{
    Operation op;
    char username[MAX_USR_SIZE];
    char data[MAX_MSG_SIZE];
} Message;

void client_server(struct sockaddr *client_addr, struct sockaddr *server_addr)
{
    int sockfd;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    Message msg;

    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));

    debug(bind(sockfd, client_addr, server_addr_len));

    memset(&msg, 0, sizeof(Message));

    msg.op = REGISTER;

    printf("Inserisci il nome utente del client: ");
    fgets(msg.username, sizeof(msg.username), stdin);
    msg.username[strcspn(msg.username, "\n")] = '\0';

    printf("Inserisci la lista di file disponibili separati da uno spazio: ");
    fgets(msg.data, sizeof(msg.data), stdin);
    msg.data[strcspn(msg.data, "\n")] = '\0';

    debug(sendto(sockfd, &msg, sizeof(Message), 0, server_addr, server_addr_len));
    debug(recvfrom(sockfd, &msg, sizeof(Message), 0, server_addr, &server_addr_len));

    printf("Risposta del server: %s\n", msg.data);

    while (1)
    {

        printf("Che operazione vuoi effettuare?\nu: vedi la lista di utenti registrati\nf: vedi la lista di file condivisi da un utente\n");
        msg.op = getchar();
        getchar();

        switch (msg.op)
        {
        case LIST_FILES:
            printf("Di che utente vuoi conoscere i file che condivide?\n");
            fgets(msg.data, sizeof(msg.data), stdin);
            msg.data[strcspn(msg.data, "\n")] = '\0';
        case LIST_USERS:
            debug(sendto(sockfd, &msg, sizeof(Message), 0, server_addr, server_addr_len));
            debug(recvfrom(sockfd, &msg, sizeof(Message), 0, server_addr, &server_addr_len));
            printf("Il server ha risposto nella seguente maniera:\n%s\n", msg.data);
            break;
        case DOWNLOAD_FILE:
            // TODO: parte di download del file da un altro client
            break;
        default:
            printf("Operazione sconosciuta\n");
            break;
        }
    }
}

void client_client()
{
    // TODO: parte di upload del file
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;

    if (argc < 4 || atoi(argv[1]) == 0 || atoi(argv[3]) == 0)
    {
        fprintf(stderr, "use: %s <porta d'ascolto client> <ip server> <porta server>", argv[0]);
        return EXIT_FAILURE;
    }

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = client_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &server_addr.sin_addr);

    client_addr.sin_port = htons(atoi(argv[1]));
    client_addr.sin_addr.s_addr = INADDR_ANY;

    int pid = fork();

    if (pid == -1)
    {
        perror("fork()");
        return EXIT_FAILURE;
    }
    else if (pid == 0) // Processo figlio
    {
        client_client();
        exit(0);
    }
    else // Processo padre
    {
        client_server((struct sockaddr *)&client_addr, (struct sockaddr *)&server_addr);
        exit(0);
    }
}