/**
 * @file client.c
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
#include <unistd.h>

#define MAX_BUFFER_LEN 256
#define MAX_USERNAME_LEN 20
#define MANDATORY_ARGS 5

#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                  \
    do                                              \
    {                                               \
        int res = fun;                              \
        if (res < 0)                                \
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

/**
 * @brief Le operazioni che il client può richiedere al server, più l'operazione di uscita.
 */
typedef enum
{
    SECRET = 's',
    ADD = 'a',
    DELETE = 'd',
    READ = 'r',
    EXIT = 'e',
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

int main(int argc, char const *argv[])
{
    int sockfd = -1, n_clients_to_share;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    Message msg;
    char buffer[MAX_BUFFER_LEN];

    if (argc < MANDATORY_ARGS || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <identity> <secret> [client ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Numero di client con cui condividere il segreto
    n_clients_to_share = argc - MANDATORY_ARGS;

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));
    if (sockfd < 0)
    {
        printf("Errore nella creazione del socket\n");
        return EXIT_FAILURE;
    }

    // Inizializza msg
    memset(&msg, 0, sizeof(msg));
    // Indirizzo del server
    memset(&server_addr, 0, server_addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // Setup con il server
    // Copio il nome utente nella struttura Message
    strncpy(msg.username, argv[3], MAX_USERNAME_LEN);

    // Primo passo: inviare il segreto
    msg.op = SECRET;
    strncpy(msg.data, argv[4], MAX_USERNAME_LEN);
    debug(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, server_addr_len));
    debug(recvfrom(sockfd, buffer, MAX_BUFFER_LEN - 1, 0, (struct sockaddr *)&server_addr, &server_addr_len));
    printf("%s\n", buffer);

    // Secondo passo: inviare la lista di client con cui condividere il segreto
    msg.op = ADD;
    for (int i = 0; i < n_clients_to_share; i++)
    {
        strncpy(msg.data, argv[MANDATORY_ARGS + i], MAX_USERNAME_LEN);
        debug(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, server_addr_len));
        debug(recvfrom(sockfd, buffer, MAX_BUFFER_LEN - 1, 0, (struct sockaddr *)&server_addr, &server_addr_len));
        printf("%s\n", buffer);
    }

    while (1)
    {
        // Lettura dell'operazione da eseguire
        printf("Inserisci l'operazione da eseguire:\n"
               "s: aggiorna il segreto\n"
               "a: aggiungi un client con cui condividere il segreto\n"
               "d: rimuovi un client con cui condividere il segreto\n"
               "r: leggi il segreto di un client\n"
               "e: esci\n");

        // Lettura dell'operazione da eseguire senza scanf
        msg.op = getchar();
        getchar(); // Consumo il carattere \n

        switch (msg.op)
        {
        case SECRET:
            printf("Inserisci il nuovo segreto: ");
            break;
        case ADD:
            printf("Inserisci il nome del client da aggiungere: ");
            break;
        case DELETE:
            printf("Inserisci il nome del client da rimuovere: ");
            break;
        case READ:
            printf("Inserisci il nome del client di cui vuoi conoscere il segreto: ");
            break;
        case EXIT:
            printf("Uscita...\n");
            close(sockfd);
            exit(EXIT_SUCCESS);
        default:
            printf("Operazione non valida\n");
            continue;
        }

        fgets(msg.data, MAX_USERNAME_LEN, stdin);
        msg.data[strcspn(msg.data, "\n")] = '\0'; // Elimino il carattere \n
        debug(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, server_addr_len));

        // Ricezione del messaggio di risposta
        debug(recvfrom(sockfd, buffer, MAX_BUFFER_LEN - 1, 0, (struct sockaddr *)&server_addr, &server_addr_len));
        printf("%s\n", buffer);
    }

    return 0;
}
