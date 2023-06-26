/**
 * 5 macchine virtuali (C1, C2, C3, C4, C5) condividono con un Server un segreto di lunghezza
 * massima di 50 caratteri. Ogni client invierà al server un elenco di macchine virtuali con cui
 * condividere tale informazione. Una generica macchina X, dopo essersi autenticata, chiederà al
 * Server di conoscere il segreto di una determinata macchina Y. Se la macchina X risulta essere
 * presente nella lista delle macchine Y allora il Server invierà il messaggio segreto, altrimenti verrà
 * restituito un messaggio di errore. La macchina X visualizzerà su schermo il messaggio ricevuto
 * (qualsiasi esso sia).
 * Una generica macchina X potrà decidere di aggiornare la lista delle macchine con cui condividere
 * tale segreto e/o modificare il contenuto stesso del messaggio.
 * Il server gestisce tutte le informazioni attraverso file di testo.
 */
#include <stdlib.h>
#include <stdio.h>
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

#define USERNAME_SIZE 21
#define SECRET_SIZE 51
#define VM_LIST_SIZE 128
#define RESPONSE_SIZE 1024

typedef enum
{
    LOGIN = 'l',
    UPDATE_LIST = 'u',
    UPDATE_SECRET = 's',
    GET_SECRET = 'g',
    EXIT = 'e',
} Operation;

typedef struct
{
    Operation op;
    char secret[SECRET_SIZE];
    char vm_list[VM_LIST_SIZE];
    char username[USERNAME_SIZE];
} Message;

int main(int argc, char const *argv[])
{
    int sockfd, n;
    struct sockaddr_in6 server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    Message msg;
    char buf[RESPONSE_SIZE];

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, len);

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[2]));
    debug(inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr));

    debug(sockfd = socket(AF_INET6, SOCK_DGRAM, 0));

    while (1)
    {
        printf("Seleziona l'operazione: \n");
        printf("l - Login\n");
        printf("u - Update list\n");
        printf("s - Update secret\n");
        printf("g - Get secret\n");
        printf("e - Exit\n");

        msg.op = getchar();
        while (getchar() != '\n')
        {
        }

        switch (msg.op)
        {
        case LOGIN:
            printf("Inserisci il tuo username: ");
            fgets(msg.username, USERNAME_SIZE, stdin);
            msg.username[strlen(msg.username) - 1] = '\0';
            printf("Inserisci la lista delle macchine virtuali con cui condividere il segreto: ");
            fgets(msg.vm_list, VM_LIST_SIZE, stdin);
            msg.vm_list[strlen(msg.vm_list) - 1] = '\0';
            printf("Inserisci il segreto: ");
            fgets(msg.secret, SECRET_SIZE, stdin);
            msg.secret[strlen(msg.secret) - 1] = '\0';
            break;
        case UPDATE_LIST:
            printf("Inserisci la lista delle macchine virtuali con cui condividere il segreto: ");
            fgets(msg.vm_list, VM_LIST_SIZE, stdin);
            msg.vm_list[strlen(msg.vm_list) - 1] = '\0';
            break;
        case UPDATE_SECRET:
            printf("Inserisci il segreto: ");
            fgets(msg.secret, SECRET_SIZE, stdin);
            msg.secret[strlen(msg.secret) - 1] = '\0';
            break;
        case GET_SECRET:
            printf("Inserisci il nome della macchina virtuale a cui chiedere il segreto: ");
            fgets(msg.vm_list, VM_LIST_SIZE, stdin);
            msg.vm_list[strlen(msg.vm_list) - 1] = '\0';
            break;
        case EXIT:
            close(sockfd);
            exit(EXIT_SUCCESS);
        default:
            printf("Operazione non valida\n");
            continue;
        }

        if (strlen(msg.username) == 0)
        {
            printf("Devi prima effettuare il login\n");
            continue;
        }

        debug(sendto(sockfd, &msg, sizeof(Message), 0, (struct sockaddr *)&server_addr, len));
        debug(n = recvfrom(sockfd, buf, RESPONSE_SIZE - 1, 0, (struct sockaddr *)&server_addr, &len));
        buf[n] = '\0';

        printf("Risposta dal server:\n%s\n", buf);
    }

    return 0;
}
