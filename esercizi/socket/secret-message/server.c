/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief 5 macchine virtuali (C1, C2, C3, C4, C5) condividono con un Server un segreto di lunghezza
 * massima di 50 caratteri. Ogni client invierà al server un elenco di macchine virtuali con cui
 * condividere tale informazione. Una generica macchina X, dopo essersi autenticata, chiederà al
 * Server di conoscere il segreto di una determinata macchina Y. Se la macchina X risulta essere
 * presente nella lista delle macchine Y allora il Server invierà il messaggio segreto, altrimenti verrà
 * restituito un messaggio di errore. La macchina X visualizzerà su schermo il messaggio ricevuto
 * (qualsiasi esso sia).
 * Una generica macchina X potrà decidere di aggiornare la lista delle macchine con cui condividere
 * tale segreto e/o modificare il contenuto stesso del messaggio.
 * Il server gestisce tutte le informazioni attraverso file di testo.
 * @version 0.1
 * @date 2023-05-27
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>

#define RESP_SUCCESS "La richiesta e' stata presa in carico dal server"
#define RESP_SECRET_SUCCESS "Il segreto e' stato aggiornato"
#define RESP_ADD_SUCCESS "L'utente e' stato aggiunto alla lista di utenti con cui condividere il segreto"
#define RESP_DELETE_SUCCESS "L'utente e' stato rimosso dalla lista di utenti con cui condividere il segreto"
#define RESP_AUTH_FAIL "Non sei autorizzato ad accedere al segreto"
#define RESP_FAIL "Si e' verificato un errore"
#define MAX_BUFFER_LEN 256
#define MAX_USERNAME_LEN 20

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

/**
 * @brief Le operazioni che il client può richiedere al server, più l'operazione di uscita.
 */
typedef enum
{
    SECRET = 's',
    ADD = 'a',
    DELETE = 'd',
    READ = 'r',
} Operation;

/**
 * @brief Struttura che rappresenta un messaggio inviato dal client al server.
 */
typedef struct
{
    Operation op;
    char username[MAX_USERNAME_LEN];
    /**
     * @brief
     * - Se op == SECRET, data contiene il messaggio segreto da salvare nel server.
     * - Se op == ADD, data contiene un nuovo utente da aggiungere alla lista di utenti con cui
     * condivide il segreto.
     * - Se op == DELETE, data contiene un utente da rimuovere dalla lista di utenti con cui
     * condivide il segreto.
     * - Se op == READ, data contiene l'utente di cui si vuole conoscere il segreto.
     */
    char data[MAX_USERNAME_LEN];
} Message;

/**
 * @brief Crea o sovrascrive il file contenente il segreto dell'utente e lo aggiorna con il nuovo segreto
 * @param user utente che vuole aggiornare il proprio segreto
 * @param secret nuovo segreto
 * @return true se l'operazione è andata a buon fine, false altrimenti
 */
bool update_secret(const char user[MAX_USERNAME_LEN], const char secret[MAX_USERNAME_LEN])
{
    // Andiamo a comporre il nome del file che contiene il segreto dell'utente
    // <user>.secret
    char file_name[MAX_USERNAME_LEN + 8];
    strcpy(file_name, user);
    strcat(file_name, ".secret");

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        perror("fopen");
        return false;
    }
    fprintf(fp, "%s", secret);
    fclose(fp);
    return true;
}

/**
 * @brief Legge il segreto dell'utente indicato dal file
 * @param user utente di cui leggere il segreto
 * @param secret buffer in cui salvare il segreto
 * @return true se l'operazione è andata a buon fine, false altrimenti
 */
bool read_secret(const char user[MAX_USERNAME_LEN], char secret[MAX_USERNAME_LEN])
{
    // Andiamo a comporre il nome del file che contiene il segreto dell'utente
    // <user>.secret
    char file_name[MAX_USERNAME_LEN + 8];
    strcpy(file_name, user);
    strcat(file_name, ".secret");

    if (access(file_name, F_OK) == -1)
    {
        // Il file non esiste
        return false;
    }

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        perror("fopen");
        return false;
    }
    fgets(secret, MAX_USERNAME_LEN, fp);
    secret[strcspn(secret, "\n")] = '\0';
    fclose(fp);
    return true;
}

/**
 * @brief Aggiunge un utente alla lista di utenti con su sta condividendo il segreto
 * @param user utente che possiede il segreto
 * @param share utente con cui si vuole condividere il segreto
 * @return true se l'operazione è andata a buon fine, false altrimenti
 */
bool add_share(const char user[MAX_USERNAME_LEN], const char share[MAX_USERNAME_LEN])
{
    // Andiamo a comporre il nome del file che contiene la lista di utenti con cui condividere il segreto
    // <user>.share
    char file_name[MAX_USERNAME_LEN + 7];
    strcpy(file_name, user);
    strcat(file_name, ".share");

    FILE *fp = fopen(file_name, "a");
    if (fp == NULL)
    {
        perror("fopen");
        return false;
    }
    fprintf(fp, "%s\n", share);
    fclose(fp);
    return true;
}

/**
 * @brief Rimuove un utente dalla lista di utenti con su sta condividendo il segreto
 * @param user utente che possiede il segreto
 * @param share utente con cui si vuole smettere di condividere il segreto
 * @return true se l'operazione è andata a buon fine, false altrimenti
 */
bool remove_share(const char user[MAX_USERNAME_LEN], const char share[MAX_USERNAME_LEN])
{
    // Andiamo a comporre il nome del file che contiene la lista di utenti con cui condividere il segreto
    // <user>.share
    char file_name[MAX_USERNAME_LEN + 7];
    strcpy(file_name, user);
    strcat(file_name, ".share");

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        perror("fopen");
        return false;
    }

    // Creiamo un file temporaneo in cui copiare tutti gli utenti tranne quello che vogliamo rimuovere
    char tmp_file_name[MAX_USERNAME_LEN + 7];
    strcpy(tmp_file_name, user);
    strcat(tmp_file_name, ".tmp");
    FILE *tmp_fp = fopen(tmp_file_name, "w");
    if (tmp_fp == NULL)
    {
        perror("fopen");
        return false;
    }

    char line[MAX_USERNAME_LEN];
    while (fgets(line, MAX_USERNAME_LEN, fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, share) != 0)
            fprintf(tmp_fp, "%s\n", line);
    }
    fclose(fp);
    fclose(tmp_fp);

    // Sostituiamo il file originale con quello temporaneo
    remove(file_name);
    rename(tmp_file_name, file_name);
    return true;
}

/**
 * @brief Controlla se l'utente share è presente nella lista di utenti con cui si sta condividendo il segreto
 * @param user utente che possiede il segreto
 * @param share utente con cui si vuole controllare se si sta condividendo il segreto
 * @return true se l'utente è presente nella lista, false altrimenti o in caso di errore
 */
bool is_shared(const char user[MAX_USERNAME_LEN], const char share[MAX_USERNAME_LEN])
{
    char file_name[MAX_USERNAME_LEN + 7];
    strcpy(file_name, user);
    strcat(file_name, ".share");

    if (access(file_name, F_OK) == -1)
    {
        // Il file non esiste
        return false;
    }

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        perror("fopen");
        return false;
    }

    // Si legge il file riga per riga e si controlla se l'utente è presente
    char line[MAX_USERNAME_LEN];
    while (fgets(line, MAX_USERNAME_LEN, fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, share) == 0)
        {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(server_addr);
    Message msg;

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Setup della struct msg
    memset(&msg, 0, sizeof(msg));

    // Creazione socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));

    // Inizializzazione indirizzo server
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Binding socket - indirizzo server
    debug(bind(sockfd, (struct sockaddr *)&server_addr, addrlen));

    // Ricezione richieste
    while (true)
    {
        char *server_response;
        char secret[MAX_USERNAME_LEN];

        debug(recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &addrlen));

        switch (msg.op)
        {
        case SECRET:
            server_response = update_secret(msg.username, msg.data) ? RESP_SECRET_SUCCESS : RESP_FAIL;
            break;
        case READ:
            if (strcmp(msg.username, msg.data) == 0 || is_shared(msg.data, msg.username))
            {
                read_secret(msg.data, secret);
                server_response = secret;
            }
            else
                server_response = RESP_AUTH_FAIL;
            break;
        case ADD:
            if (strcmp(msg.username, msg.data) == 0 || is_shared(msg.data, msg.username))
                server_response = RESP_ADD_SUCCESS;
            else
                server_response = add_share(msg.username, msg.data) ? RESP_ADD_SUCCESS : RESP_FAIL;
            break;
        case DELETE:
            if (strcmp(msg.username, msg.data) == 0 || !is_shared(msg.data, msg.username))
                server_response = RESP_DELETE_SUCCESS;
            else
                server_response = remove_share(msg.username, msg.data) ? RESP_DELETE_SUCCESS : RESP_FAIL;
            break;
        default:
            server_response = RESP_FAIL;
            break;
        }

        debug(sendto(sockfd, server_response, strlen(server_response) + 1, 0, (struct sockaddr *)&client_addr, addrlen));
    }

    close(sockfd);
    return 0;
}
