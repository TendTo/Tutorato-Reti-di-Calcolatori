/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Server che implementa il protocollo NTP.
 * Il server riceverà un messaggio dal client che indica la sua volontà di registrarsi al servizio.
 * Per 30 secondi dopo la registrazione, il server invierà al client data e ora correnti, ogni 5 secondi.
 * Allo scadere dei 30 secondi, la registrazione scadrà e dovrà essere rinnovata.
 * @version 0.1
 * @date 2023-05-22
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
#include <time.h>
#include <pthread.h>

#define MAX_N_CLIENTS 10
#define MAX_BUFFER_LEN 100
#define SERVICE_LEN_SEC 30
#define RESP_SUCCESS "La richiesta è stata presa in carico dal server"
#define RESP_FAIL "Il server è sovraccarico"
#define RESP_DEL "Servizio terminato"

struct client_registration_str
{
    struct sockaddr_in addr;
    time_t creation_timestamp;
};

// Clienti registrati attualmente registrati al servizio
struct client_registration_str clients[MAX_N_CLIENTS];
// Protezione dei client dalla concorrenza tramite mutex
pthread_mutex_t mutexdata;

/**
 * @brief Aggiunge un nuovo client nella lista.
 * Se la lista è piena, nessun utente viene aggiunto
 * @param client_addr indirizzo del client che verrà notificato
 * @return 1 se il client è stato aggiunto, 0 altrimenti
 */
int add_client(struct sockaddr_in *client_addr)
{
    pthread_mutex_lock(&mutexdata);
    for (int i = 0; i < MAX_N_CLIENTS; i++)
    {
        if (clients[i].creation_timestamp == 0)
        {
            clients[i].addr = *client_addr;
            clients[i].creation_timestamp = time(NULL);
            printf("Created new client: timestamp %ld\n", clients[i].creation_timestamp);
            pthread_mutex_unlock(&mutexdata);
            return 1;
        }
    }
    pthread_mutex_unlock(&mutexdata);
    return 0;
}

/**
 * @brief Thread che ogni 5 secondi notifica tutti i client iscritti con l'ora corrente.
 * Se il servizio è scaduto, rimuove il client dalla lista di quelli attivi
 * @param sockfd descrittore della socket da usare
 */
void notify_clients(const int *sockfd)
{
    char buffer[MAX_BUFFER_LEN];
    struct tm *tm_info;
    time_t timer;

    printf("Thread started\n");

    while (1)
    {
        timer = time(NULL);
        tm_info = localtime(&timer);
        strftime(buffer, MAX_BUFFER_LEN, "%Y-%m-%d %H:%M:%S", tm_info);
        pthread_mutex_lock(&mutexdata);
        for (int i = 0; i < MAX_N_CLIENTS; i++)
        {
            if (clients[i].creation_timestamp == 0)
            {
                continue;
            }
            else if (clients[i].creation_timestamp + SERVICE_LEN_SEC < timer)
            {
                printf("Deleting old client, index %d\n", i);
                clients[i].creation_timestamp = 0;
                if (sendto(*sockfd, RESP_DEL, strlen(RESP_DEL) + 1, 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr)) < 0)
                {
                    perror("thread - sendto()");
                    pthread_exit((void *)EXIT_FAILURE);
                }
            }
            else if (clients[i].creation_timestamp != 0)
            {
                printf("Seding time to client: id %d\n", i);
                if (sendto(*sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr)) < 0)
                {
                    perror("thread - sendto()");
                    pthread_exit((void *)EXIT_FAILURE);
                }
            }
        }
        pthread_mutex_unlock(&mutexdata);
        sleep(5);
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    char buffer[MAX_BUFFER_LEN];
    char *server_resp;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sock_len = sizeof(struct sockaddr_in);

    memset(&server_addr, 0, sock_len);
    memset(&client_addr, 0, sock_len);
    memset(&clients, 0, sizeof(clients));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, sock_len) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    // Avvia il thread per la notifica dei client e inizializza il mutex
    printf("Starting thread\n");
    pthread_t thread;
    pthread_mutex_init(&mutexdata, NULL);
    if (pthread_create(&thread, NULL, (void *(*)(void *))notify_clients, (void *)&sockfd) < 0)
    {
        perror("thread_create()");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Attende un'inscizione da parte di un client
        if (recvfrom(sockfd, buffer, MAX_BUFFER_LEN, 0, (struct sockaddr *)&client_addr, &sock_len) < 0)
        {
            perror("recvfrom()");
            exit(EXIT_FAILURE);
        }

        // Notifica il client con il risultato dell'iscrizione
        printf("Received message from client\n");
        server_resp = add_client(&client_addr) ? RESP_SUCCESS : RESP_FAIL;

        if (sendto(sockfd, server_resp, strlen(server_resp) + 1, 0, (struct sockaddr *)&client_addr, sock_len) < 0)
        {
            perror("sendto()");
            exit(EXIT_FAILURE);
        }
    }

    pthread_cancel(thread);
    pthread_mutex_destroy(&mutexdata);
    close(sockfd);

    return 0;
}
