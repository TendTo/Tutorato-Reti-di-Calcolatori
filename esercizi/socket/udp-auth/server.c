/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Server che si occupa dell'autenticazione.
 * Verrà creato un file di testo 'database.txt' che conterrà le coppie username,password.
 * Gli utenti si possono registrare al server, a patto che il loro username sia univoco.
 * Il login verifica che l'utente sia registrato e che la password sia corretta.
 * Il delete elimina l'utente dal database, a patto che sia registrato e che username e password siano corretti.
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
#define MAX_BUFFER_SIZE 1000
#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20
#define MAX_LINE_SIZE MAX_PASSWORD_SIZE + 1 + MAX_USERNAME_SIZE
#define RESP_REGISTER_SUCCESS "Registrazione effettuata con successo!"
#define RESP_REGISTER_ERROR "L'utente è già registrato"
#define RESP_LOGIN_SUCCESS "Login effettuato con successo! Benvenuto!"
#define RESP_LOGIN_ERROR "Username o password errati"
#define RESP_DELETE_SUCCESS "Utente eliminato con successo!"
#define RESP_DELETE_ERROR "L'utente non è registrato"
#define ILLEGAL_CHAR_MESSAGE "Il simbolo '" SEPARATOR "' non può essere usato all'intero di nome utente o password"

typedef struct
{
    char username[MAX_USERNAME_SIZE];
    char password[MAX_PASSWORD_SIZE];
} User;

typedef enum
{
    REGISTER = 'r',
    LOGIN = 'l',
    DELETE = 'd'
} Operation;

typedef struct
{
    Operation op;
    User u;
} Message;

/**
 * @brief Effettua il login di un utente con username e password.
 * Se password è NULL, controlla se l'utente è già presente nel file utilizzando il suo username.
 *
 * @param username username dell'utente da cercare
 * @param password password dell'utente che vuole fare il login. Se NULL, si verifica solo se l'utente è presente
 * @param file file in cui cercare l'utente
 * @return true se l'utente è presente, false altrimenti
 */
bool login_user(const char username[], const char password[], FILE *f)
{
    char riga[MAX_LINE_SIZE];

    if (password != NULL)
        printf("Tentativo di login di %s,%s\n", username, password);
    else
        printf("Ricerca utente %s\n", username);

    rewind(f);
    while (fgets(riga, sizeof(riga), f) != NULL)
    {
        // Taglia la riga al primo separatore
        int sep_pos = strcspn(riga, SEPARATOR);
        riga[strcspn(riga, "\n")] = '\0';
        riga[sep_pos] = '\0';

        char *r_username = riga;
        char *r_password = riga + sep_pos + 1;
        if (strcmp(r_username, username) == 0 && (password == NULL || strcmp(r_password, password) == 0))
            return true;
    }
    return false;
}

/**
 * @brief Prova a registrare un nuovo utente.
 * Se questo è già presente, non viene effettuata alcuna operazione.
 *
 * @param u utente che vuole registrarsi
 * @param f file in cui registrare l'utente
 * @return true se la registrazione va a buon fine, false altrimenti
 */
bool register_user(User *u, FILE *f)
{
    // L'utente è già registrato
    if (login_user(u->username, NULL, f))
        return false;

    printf("Registrazione utente %s\n", u->username);

    // L'utente viene registrato alla fine del file con un formato "username,password"
    fseek(f, 0, SEEK_END);
    fprintf(f, "%s" SEPARATOR "%s\n", u->username, u->password);
    fflush(f);
    return true;
}

/**
 * @brief Gestisce la cancellazione di un utente.
 *
 * @param u utente da cancellare
 * @param f file in cui cancellare l'utente
 * @return true se l'utente viene cancellato, false altrimenti
 */
bool delete_user(User *u, FILE *f)
{
    // Se l'utente non è registrato, non può essere cancellato
    if (!login_user(u->username, u->password, f))
        return false;

    printf("Rimozione utente %s\n", u->username);
    // Crea un file temporaneo, copiando tutti gli utenti tranne quello da cancellare
    FILE *tmp = tmpfile();

    rewind(f);
    char riga[MAX_LINE_SIZE];

    while (fgets(riga, MAX_LINE_SIZE, f) != NULL)
    {
        char username[MAX_USERNAME_SIZE];
        strncpy(username, riga, MAX_USERNAME_SIZE);
        username[strcspn(username, SEPARATOR)] = '\0';
        if (strcmp(username, u->username) != 0)
            fprintf(tmp, "%s", riga);
    }
    fflush(tmp);

    // Cancella il contenuto del database
    FILE *f2 = fopen("database.txt", "w");
    fclose(f2);

    // Sostituisce il file originale con quello temporaneo
    rewind(tmp);
    rewind(f);
    while (fgets(riga, MAX_LINE_SIZE, tmp) != NULL)
        fprintf(f, "%s", riga);

    fflush(f);

    // Chiude il file temporaneo
    fclose(tmp);

    return true;
}

/**
 * @brief Gestisce l'operazione richiesta dal client.
 *
 * @param msg messaggio ricevuto dal client
 * @param f file del database
 * @return stringa di risposta da inviare al client
 */
char *handle_operation(Message *msg, FILE *f)
{
    switch (msg->op)
    {
    case REGISTER:
        return register_user(&msg->u, f) ? RESP_REGISTER_SUCCESS : RESP_REGISTER_ERROR;
    case LOGIN:
        return login_user(msg->u.username, msg->u.password, f) ? RESP_LOGIN_SUCCESS : RESP_LOGIN_ERROR;
    case DELETE:
        return delete_user(&msg->u, f) ? RESP_DELETE_SUCCESS : RESP_DELETE_ERROR;
    default:
        break;
    }
}

/**
 * @brief Assicura che l'input non contenga caratteri non validi.
 * @param input input da controllare
 * @return true se l'input è valido, false altrimenti
 */
bool validate_input(const char input[])
{
    return (strstr(input, SEPARATOR) == NULL &&
            strchr(input, '\n') == NULL);
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t sock_len = sizeof(struct sockaddr_in6);
    socklen_t client_sock_len = sock_len;
    Message msg;

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
    // recvfrom => c | nume utente | password
    while (1)
    {
        // Si va a salvare il messaggio ricevuto direttamente nella struttura Message
        if ((n = recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&client_addr, &client_sock_len)) > 0)
        {
            char *server_response;
            if (!validate_input(msg.u.username) || !validate_input(msg.u.password))
                server_response = ILLEGAL_CHAR_MESSAGE;
            else
                server_response = handle_operation(&msg, f);

            if (sendto(sockfd, server_response, strlen(server_response) + 1, 0, (struct sockaddr *)&client_addr, client_sock_len) < 0)
                perror("Errore nella risposta al client");
        }
    }

    fclose(f);
    close(sockfd);
    return 0;
}
