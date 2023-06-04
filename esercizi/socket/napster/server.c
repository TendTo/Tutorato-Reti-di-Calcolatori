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
#define PING_INTERVAL 20 // Intervallo di tempo in secondi tra un ping e l'altro
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

    printf("Sto registrando l'utente %s con ip %s e porta %d. Offre i file %s\n", msg->username, ip, msg->port, msg->data);

    FILE *database = fopen(DATABASE_FILE, "a");
    fprintf(database, "%s %s %d %s\n", msg->username, ip, msg->port, msg->data);
    fclose(database);

    strcpy(msg->data, "Utente registrato con successo");
}

/**
 * @brief Aggiorna la lista di file condivisi da un utente in risposta ad un messaggio ricevuto dall'utente stesso.
 * La nuova lista di file, presente nella sezione 'data' del messaggio, viene salvata nel database al posto di quella vecchia.
 * La riga aggiunta avrà questo formato:
 * <username> <ip> <porta> <lista di file separati da spazio>
 * @param msg messaggio ricevuto dal client che contiene le informazioni da salvare nel file
 */
void update_files(Message *msg)
{
    char line[MAX_MSG_SIZE];
    char line_username[MAX_USR_SIZE];
    FILE *database = fopen(DATABASE_FILE, "r");
    FILE *new_database = fopen("new_database.txt", "w");

    while (fgets(line, MAX_MSG_SIZE, database) != NULL)
    {
        // Ottiene il nome utente dalla riga
        memset(line_username, 0, MAX_USR_SIZE);
        printf("Line: %s\n", line);
        strncpy(line_username, line, strcspn(line, SEPARATOR));
        printf("Username: %s\n", line_username);
        // Se il nome utente è quello del client che ha inviato il messaggio, aggiorna la lista di file
        if (strcmp(line_username, msg->username) == 0)
        {
            char *username = strtok(line, SEPARATOR);
            char *ip = strtok(NULL, SEPARATOR);
            char *port = strtok(NULL, SEPARATOR);
            fprintf(new_database, "%s %s %s %s\n", username, ip, port, msg->data);
        }
        // Altrimenti scrive la riga così com'è
        else
        {
            fputs(line, new_database);
        }
    }
    // Chiusura dei file e rinomina del nuovo database
    fclose(database);
    fclose(new_database);
    remove(DATABASE_FILE);
    rename("new_database.txt", DATABASE_FILE);
    strcpy(msg->data, "Lista di file aggiornata con successo");
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

/**
 * @brief Restituisce la lista di file condivisi dall'utente specificato nella sezione 'data' del messaggio.
 * Questi vengono inseriti nella sezione 'data' del messaggio, e saranno inviati al client che ha fatto la richiesta.
 * Le righe nel database sono nel formato:
 * <username> <ip> <porta> <lista di file separati da spazio>
 * @param msg messaggio ricevuto dal client
 */
void list_files(Message *msg)
{
    char line[MAX_MSG_SIZE];
    FILE *database = fopen(DATABASE_FILE, "r");
    char *username;

    while (fgets(line, MAX_MSG_SIZE, database) != NULL)
    {
        username = strtok(line, SEPARATOR);
        if (strcmp(username, msg->data) == 0)
        {
            memset(msg->data, '\0', sizeof(msg->data));
            strtok(NULL, SEPARATOR); // ip
            strtok(NULL, SEPARATOR); // porta

            while ((username = strtok(NULL, SEPARATOR)) != NULL) // Scorri ogni file nella lista
            {
                strcat(msg->data, username);  // Concatena il nome del file
                strcat(msg->data, SEPARATOR); // Concatena uno spazio
            }
            // Rimuovi l'ultimo spazio
            if (strlen(msg->data) > 0)
                msg->data[strlen(msg->data) - 1] = '\0';
            fclose(database);
            return;
        }
    }

    // Se l'utente non è stato trovato
    strcpy(msg->data, "Utente non trovato");
    fclose(database);
}

/**
 * @brief Restituisce le informazioni (ip e porta) dell'utente specificato nella sezione 'data' del messaggio.
 * Queste vengono inserite nella sezione 'data' del messaggio, e saranno inviate al client che ha fatto la richiesta.
 * Le righe nel database sono nel formato:
 * <username> <ip> <porta> <lista di file separati da spazio>
 * @param msg messaggio ricevuto dal client
 */
void info_user(Message *msg)
{
    char line[MAX_MSG_SIZE];
    FILE *database = fopen(DATABASE_FILE, "r");
    char *username;

    while (fgets(line, MAX_MSG_SIZE, database) != NULL)
    {
        username = strtok(line, SEPARATOR);
        if (strcmp(username, msg->data) == 0)
        {
            memset(msg->data, '\0', sizeof(msg->data));
            strcat(msg->data, strtok(NULL, SEPARATOR)); // ip
            msg->port = atoi(strtok(NULL, SEPARATOR));  // porta
            fclose(database);
            return;
        }
    }

    // Se l'utente non è stato trovato
    strcpy(msg->data, "Utente non trovato");
    msg->port = 0;
    fclose(database);
}

void ping_users()
{
    while (1)
    {
        sleep(PING_INTERVAL); // Aspetta PING_INTERVAL secondi
        printf("Pingando gli utenti\n");
        FILE *database = fopen(DATABASE_FILE, "r");

        char line[MAX_MSG_SIZE];
        char *username, *ip, *port, *files;
        int sockfd;
        struct sockaddr_in client_addr;
        char pong[sizeof(PONG_STRING)];

        while (fgets(line, MAX_MSG_SIZE, database) != NULL)
        {
            username = strtok(line, SEPARATOR);
            ip = strtok(NULL, SEPARATOR);
            port = strtok(NULL, SEPARATOR);

            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            memset(&client_addr, 0, sizeof(client_addr));
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(atoi(port));
            inet_pton(AF_INET, ip, &client_addr.sin_addr);

            printf("Pingando l'utente '%s' all'indirizzo %s:%s\n", username, ip, port);

            if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
            {
                printf("L'utente '%s' non è raggiungibile\n", username);
                continue;
            }
            debug(send(sockfd, PING_STRING, sizeof(PING_STRING), 0));
            debug(recv(sockfd, pong, sizeof(pong), 0));
            if (strcmp(pong, PONG_STRING) != 0)
                printf("L'utente '%s' non ha risposto al ping\n", username);
            else
                printf("L'utente '%s' ha risposto al ping\n", username);
        }
    }
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

    if (fork() == 0)
    {
        ping_users();
        exit(EXIT_SUCCESS);
    }

    while (1)
    {
        debug(recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, &client_struct_length));

        switch (msg.op)
        {
        case REGISTER:
            register_user(&msg, &client_addr);
            break;
        case LIST_USERS:
            list_users(&msg);
            break;
        case LIST_FILES:
            list_files(&msg);
            break;
        case INFO_USER:
            info_user(&msg);
            break;
        case UPDATE_FILES:
            update_files(&msg);
            break;
        default:
            printf("Operazione non riconosciuta: %c\n", msg.op);
            continue;
        }
        // Rispondi al client con il messaggio opportunamente aggiornato
        sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, client_struct_length);
    }
}