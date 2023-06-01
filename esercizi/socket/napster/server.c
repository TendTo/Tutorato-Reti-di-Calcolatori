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
#define DATABASE_FILE "database.txt"
#define SEPARATOR " "

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

/**
 * @brief Aggiunge un nuovo utente alla lista di quelli registrati nel database.
 * La riga aggiunta avrà questo formato:
 * <username> <ip> <porta> <lista di file separati da spazio>
 * @param msg messaggio ricevuto dal client che contiene le informazioni da salvare nel file
 */
void register_user(Message *msg, struct sockaddr_in *client_addr)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, ip, sizeof(ip));
    int port = ntohs(client_addr->sin_port);
    printf("Sto registrando l'utente %s con ip %s e porta %d. Offre i file %s\n", msg->username, ip, port, msg->data);

    FILE *database = fopen(DATABASE_FILE, "a");
    fprintf(database, "%s %s %d %s\n", msg->username, ip, port, msg->data);
    fclose(database);
}

/**
 * @brief Scorre il database e mette tutti gli username che incontra diversi da quello di chi ha chiesto
 * in una stringa di risposta da inviare al chiesitore.
 * Le righe nel database sono nel formato:
 * <username> <ip> <porta> <lista di file separati da spazio>
 * @param msg messaggio ricevuto dal client
 */
void list_users(Message *msg)
{
    char line[MAX_MSG_SIZE];
    FILE *database = fopen(DATABASE_FILE, "r");
    char *username;
    memset(msg->data, '\0', sizeof(msg->data));

    while (fgets(line, MAX_MSG_SIZE, database) != NULL)
    {
        username = strtok(line, SEPARATOR);
        if (strcmp(username, msg->username) != 0)
        {
            strcat(msg->data, username);
            strcat(msg->data, " ");
        }
    }
    if (strlen(msg->data) > 0)
        msg->data[strlen(msg->data) - 1] = '\0';

    fclose(database);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_struct_length = sizeof(client_addr);
    Message msg;

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <porta d'ascolto>", argv[0]);
        return EXIT_FAILURE;
    }

    // Crea il file se non esiste
    FILE *database = fopen(DATABASE_FILE, "a");
    fclose(database);

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));

    debug(bind(sockfd, (struct sockaddr *)&server_addr, client_struct_length));

    while (1)
    {
        debug(recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, &client_struct_length));

        switch (msg.op)
        {
        case REGISTER:
            register_user(&msg, &client_addr);
            strcpy(msg.data, "Utente registrato con successo");
            sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, client_struct_length);
            break;
        case LIST_USERS:
            list_users(&msg);
            sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, client_struct_length);
            break;
        case INFO_USER:
            // TODO: fornire al client le informazioni di un altro client (ip e porta)
            break;
        default:
            break;
        }
    }
}