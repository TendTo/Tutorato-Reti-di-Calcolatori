/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Semplice server tcp ipv4
 * @version 0.1
 * @date 2023-05-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>

#define SERVER_RESP "Job completed!"
#define MAX_MSG_SIZE 1000
#define PORT 2000
#define N_ACCEPT 5
#define REPETITIONS 2

void child_process(int sockfd_conn, struct sockaddr_in *client_addr)
{
    char client_message[MAX_MSG_SIZE], server_message[MAX_MSG_SIZE];

    printf("[>] Listening for incoming messages...\n");
    if (recv(sockfd_conn, client_message, sizeof(client_message), 0) < 0)
    {
        perror("Couldn't receive");
        return;
    }

    printf("[>] Msg from client: %s\n", client_message);

    // Simuliamo un'operazione che richiede un po' di tempo per essere svolta
    sleep(10);

    // Si invia il messaggio al client
    if (send(sockfd_conn, SERVER_RESP, strlen(SERVER_RESP) + 1, 0) < 0)
    {
        perror("Can't send");
        return;
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char server_message[MAX_MSG_SIZE], client_message[MAX_MSG_SIZE];
    socklen_t client_struct_length = sizeof(client_addr);

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Creazione del socket.
    // Dato che upd è il protocollo di default per SOCK_STREAM, il terzo parametro può essere 0
    // sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error while creating socket");
        return 1;
    }
    printf("Socket created successfully\n");

    // Si impostano le informazioni del server
    // In questo esempio la porta è hard-coded
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Il server accetta connessioni da qualsiasi indirizzo

    // Si associa il socket all'indirizzo e alla porta
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Couldn't bind to the port");
        return -1;
    }
    printf("Done with binding\n");

    // Si è disposti ad accettare le richieste di connessione in arrivo
    if (listen(sockfd, N_ACCEPT) < 0)
    {
        perror("Could't listen to connections");
        return 1;
    }

    for (int i = 0; i < REPETITIONS; i++)
    {
        printf("I: %d\n", i);
        int sockfd_conn = accept(sockfd, (struct sockaddr *)&client_addr, &client_struct_length);
        if (sockfd_conn < 0)
        {
            perror("Couldn'd accept new connection");
            return 1;
        }

        int pid = fork();
        if (pid == -1) // Errore
        {
            perror("Error while forking");
            close(sockfd_conn);
            continue;
        }
        else if (pid == 0) // Figlio
        {
            close(sockfd);
            child_process(sockfd_conn, &client_addr);
            close(sockfd_conn);
            exit(0);
        }
        else // Padre
        {
            close(sockfd_conn);
            continue;
        }
    }

    // Attende che tutti i processi figli abbiano terminato
    // Non appena la wait va in errore, dato che non c'è più niente da aspettare, il ciclo si conclude
    printf("Waiting for children to finish\n");
    while (wait(NULL) > 0)
    {
    }

    // Si chiude la socket
    close(sockfd);

    return 0;
}
