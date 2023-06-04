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
#define PING_STRING "\nPing\n" // Stringa che il server invia al client per verificare che sia ancora attivo. Gli \n sono utili per evitare di confondere il messaggio con quello di un file da scaricare
#define PONG_STRING "\nPong\n" // Stringa che il server invia al client per verificare che sia ancora attivo. Gli \n sono utili per evitare di confondere il messaggio con quello di un file da scaricare

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
    Operation op;                // Operazione da eseguire
    char username[MAX_USR_SIZE]; // Username dell'utente
    char data[MAX_MSG_SIZE];     // Contiene il nome del file da scaricare, l'ip o il nome del client che si vuole contattare
    int port;                    // Porta di ascolto del client
} Message;

void download_file(Message *msg)
{
    struct sockaddr_in peer_addr;
    int sockfd;
    socklen_t peer_addr_len = sizeof(struct sockaddr_in);

    // Ottieni l'ip dalla risposta del server
    char ip[INET_ADDRSTRLEN];
    strncpy(ip, msg->data, INET_ADDRSTRLEN);

    printf("Quale file vuoi scaricare?\n");
    fgets(msg->data, sizeof(msg->data), stdin);
    msg->data[strcspn(msg->data, "\n")] = '\0';
    msg->op = DOWNLOAD_FILE;

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));

    // Connetti al peer con le informazioni ip e port ottenute dal server
    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    peer_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &peer_addr.sin_addr);
    peer_addr.sin_port = htons(msg->port);

    debug(connect(sockfd, (struct sockaddr *)&peer_addr, peer_addr_len));
    debug(send(sockfd, msg->data, sizeof(msg->data), 0));

    // Controlla se il file esiste già
    if (access(msg->data, F_OK) == 0)
    {
        printf("Il file '%s' esiste già, vuoi sovrascriverlo? [y/n] ", msg->data);
        char c = getchar();
        getchar();
        if (c != 'y')
            return;
    }
    // Crea il file e scrivi il contenuto
    FILE *file = fopen(msg->data, "w");
    char buffer[MAX_MSG_SIZE];
    while (recv(sockfd, buffer, MAX_MSG_SIZE, 0) > 0)
        fprintf(file, "%s", buffer);
    fclose(file);
}

void client_to_server(int port, struct sockaddr *server_addr)
{
    int sockfd;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    Message msg;

    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));

    memset(&msg, 0, sizeof(Message));

    msg.op = REGISTER;
    msg.port = port;

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

        printf("Che operazione vuoi effettuare?\nu: vedi la lista di utenti registrati\nf: vedi la lista di file condivisi da un utente\nd: download file\nl: aggiorna la lista dei file condivisi\n");
        msg.op = getchar();
        getchar();

        switch (msg.op)
        {
        case LIST_FILES:
            printf("Di che utente vuoi conoscere i file che condivide?\n");
            fgets(msg.data, sizeof(msg.data), stdin);
            msg.data[strcspn(msg.data, "\n")] = '\0';
            // Continua sotto
        case LIST_USERS:
            debug(sendto(sockfd, &msg, sizeof(Message), 0, server_addr, server_addr_len));
            debug(recvfrom(sockfd, &msg, sizeof(Message), 0, server_addr, &server_addr_len));
            printf("Il server ha risposto nella seguente maniera:\n%s\n", msg.data);
            break;
        case DOWNLOAD_FILE:
            printf("Da che utente vuoi scaricare il file?\n");
            fgets(msg.data, sizeof(msg.data), stdin);
            msg.data[strcspn(msg.data, "\n")] = '\0';
            msg.op = INFO_USER;
            debug(sendto(sockfd, &msg, sizeof(Message), 0, server_addr, server_addr_len));
            debug(recvfrom(sockfd, &msg, sizeof(Message), 0, server_addr, &server_addr_len));

            if (msg.port == 0)
            {
                printf("L'utente richiesto non e' presente sul server\n");
                break;
            }
            printf("Il server ha fornito le seguenti informazioni:\nip\t%s\nport\t%d\n", msg.data, msg.port);
            download_file(&msg);
            break;
        case UPDATE_FILES:
            printf("Inserisci la lista di file disponibili separati da uno spazio: ");
            fgets(msg.data, sizeof(msg.data), stdin);
            msg.data[strcspn(msg.data, "\n")] = '\0';
            debug(sendto(sockfd, &msg, sizeof(Message), 0, server_addr, server_addr_len));
            debug(recvfrom(sockfd, &msg, sizeof(Message), 0, server_addr, &server_addr_len));
            printf("Il server ha risposto nella seguente maniera:\n%s\n", msg.data);
            break;
        default:
            printf("Operazione sconosciuta\n");
            break;
        }
    }
}

/**
 * @brief Invia il file richiesto ad un altro client (peer)
 * @param sockfd socket tcp connessa al peer
 * @param filename nome del file da inviare
 */
void upload_file(int sockfd, char filename[])
{
    char line[MAX_MSG_SIZE];
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Il file richiesto '%s' non esiste\n", filename);
        return;
    }
    while (fgets(line, sizeof(line), file) != NULL)
        debug(send(sockfd, line, sizeof(line), 0));
    fclose(file);
}

/**
 * @brief Il client fa da server per altri client (peer)
 * Non appena riceve una richiesta per un file, il client inizia a inviarlo.
 * @param client_addr indirizzo del client pubblicato sul server
 */
void client_to_client(struct sockaddr *client_addr)
{
    int sockfd;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(bind(sockfd, client_addr, client_addr_len));
    debug(listen(sockfd, 10));

    while (1)
    {
        int peerfd;
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(struct sockaddr_in);

        debug(peerfd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_addr_len));

        int pid = fork();
        if (pid == 0)
        {
            // Processo figlio
            close(sockfd);
            char filename[MAX_MSG_SIZE];
            debug(recv(peerfd, filename, sizeof(filename), 0));
            if (strcmp(filename, PING_STRING) == 0){
                printf("Il server ha richiesto un PING\n");
                send(peerfd, PONG_STRING, sizeof(PONG_STRING), 0);
                close(peerfd);
                exit(EXIT_SUCCESS);
            }
            printf("Il client con ip '%s' ha richiesto il file '%s'\n", inet_ntoa(peer_addr.sin_addr), filename);
            upload_file(peerfd, filename);
            close(peerfd);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0)
        {
            // Processo padre
            close(peerfd);
            continue;
        }
        else
        {
            perror("fork");
            continue;
        }
    }
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
        client_to_client((struct sockaddr *)&client_addr);
        exit(0);
    }
    else // Processo padre
    {
        client_to_server(atoi(argv[1]), (struct sockaddr *)&server_addr);
        exit(0);
    }
}