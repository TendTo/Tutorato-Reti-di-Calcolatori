/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Gli studenti di informatica del corso di Reti di Calcolatori hanno deciso di implementare un server Christmas in grado di ricevere e gestire bigliettini di Natale contenenti messaggi del tipo "Oggi supererò il laboratorio di Reti".
 * Un generico client avrà la possibilità di:
 *
 * - inviare un messaggio al server
 * - fare una richiesta per ricevere tutti i messaggi già inviati (solo dello specifico client).
 * - modificare e/o cancellare un messaggio specifico.
 *
 * Il server dovrà essere progettato in modo da memorizzare qualsiasi messaggio (associandolo allo specifico client) e gestire le operazioni precedentemente elencate.
 * Quando il client invierà il carattere `q`, il programma (per lo specifico client) terminerà.
 *
 * Si utilizzino le Socket UDP per la comunicazione.
 *
 * Lo studente può definire qualsiasi approccio per risolvere l'esercizio.
 * @version 0.1
 * @date 2023-12-08
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

#include "definitions.h"

typedef struct
{
    uint64_t id;     // Identificativo del client
    char **messages; // Array di messaggi
    int n_messages;  // Numero di messaggi che il client ha inviato al server
} Client;

typedef struct
{
    int n_clients;   // Numero di client registrati
    Client *clients; // Array di client
} ServerState;

/**
 * @brief Cerca il client con l'id specificato all'interno dell'array di client.
 * Se il client non è presente e il flag insert è true,
 * questo viene aggiunto all'array, allocando la memoria necessaria.
 * Altrimenti, si limita a restituire l'indice del client all'interno dell'array.
 *
 * @param state Stato del server
 * @param id Identificativo del client da cercare o aggiungere
 * @param insert Flag che indica se aggiungere il client all'array o meno nel caso non fosse presente
 * @return Indice del client all'interno dell'array o -1 se non è stato trovato e non è stato aggiunto
 */
int getsert_client_idx(ServerState *state, uint64_t id, bool insert)
{
    for (int i = 0; i < state->n_clients; i++)
    {
        if (state->clients[i].id == id)
        {
            return i;
        }
    }
    if (!insert)
    {
        return -1;
    }

    // Inizializza l'array di client o allungalo di una posizione
    state->clients = state->n_clients == 0 ? malloc(sizeof(Client)) : realloc(state->clients, sizeof(Client) * (state->n_clients + 1));
    state->clients[state->n_clients].id = id;
    state->clients[state->n_clients].messages = NULL;
    state->clients[state->n_clients].n_messages = 0;
    return state->n_clients++;
}

/**
 * @brief Gestisci il messaggio ricevuto dal client.
 * In base all'azione indicata nel messaggio, il server può:
 * - inserire un nuovo messaggio
 * - modificare un messaggio
 * - eliminare un messaggio
 * - inviare la lista di messaggi di un client
 *
 * @param sockfd Socket del server
 * @param msg Messaggio ricevuto dal client
 * @param state Stato del server, contiene una lista di client e i loro messaggi
 * @param client_addr Indirizzo del client che ha inviato il messaggio
 */
void handle_message(int sockfd, Message *msg, ServerState *state, struct sockaddr_in *client_addr)
{
    // Questo genera un id univoco per ogni client, legando l'ip e la porta del client
    // in un unico numero intero
    // ip = 32 bit, porta = 16 bit
    // id = ip << 16 | porta = 48 bit
    long client_id = client_addr->sin_addr.s_addr;
    client_id <<= 16;
    client_id |= client_addr->sin_port;
    int client_idx = getsert_client_idx(state, client_id, true);

    printf("Client %s:%d has id %lu and is located in idx %d\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), client_id, client_idx);

    switch (msg->action)
    {
    case INSERT:
        printf("Client %s:%d inserted a message: %s\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), msg->message);
        // Inizializza l'array di messaggi o allungalo di una posizione
        state->clients[client_idx].messages = state->clients[client_idx].n_messages == 0 ? state->clients[client_idx].messages = malloc(sizeof(char *)) : realloc(state->clients[client_idx].messages, sizeof(char *) * (state->clients[client_idx].n_messages + 1));
        state->clients[client_idx].messages[state->clients[client_idx].n_messages] = strdup(msg->message);
        msg->id = state->clients[client_idx].n_messages++;
        msg->action = SUCCESS;
        break;
    case UPDATE:
        if (msg->id >= state->clients[client_idx].n_messages)
        {
            msg->action = FAIL;
            break;
        }
        printf("Client %s:%d updated a message: %s\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), msg->message);
        free(state->clients[client_idx].messages[msg->id]);
        state->clients[client_idx].messages[msg->id] = strdup(msg->message);
        msg->action = SUCCESS;
        break;
    case DELETE:
        if (msg->id >= state->clients[client_idx].n_messages)
        {
            msg->action = FAIL;
            break;
        }
        printf("Client %s:%d deleted a message: %s\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), msg->message);
        free(state->clients[client_idx].messages[msg->id]);
        state->clients[client_idx].messages[msg->id] = NULL;
        msg->action = SUCCESS;
    case LIST:
        printf("Client %s:%d requested a list of messages\n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
        msg->action = LIST;
        // Invia tutti i messaggi del client al client
        client_idx = msg->id == 0 ? client_idx : getsert_client_idx(state, msg->id, false);
        printf("Client idx: %d\n", client_idx);
        for (int i = 0; client_idx != -1 && i < state->clients[client_idx].n_messages; i++)
        {
            if (state->clients[client_idx].messages[i] != NULL)
            {
                printf("Message: %s, number: %d\n", state->clients[client_idx].messages[i], i);
                msg->id = i;
                strcpy(msg->message, state->clients[client_idx].messages[i]);
                debug(sendto(sockfd, msg, sizeof(Message), 0, (struct sockaddr *)client_addr, sizeof(struct sockaddr_in)));
            }
        }
        // Invia un messaggio di fine lista
        msg->action = END;
    }

    debug(sendto(sockfd, msg, sizeof(Message), 0, (struct sockaddr *)client_addr, sizeof(struct sockaddr_in)));
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    Message msg;
    ServerState state;
    memset(&state, 0, sizeof(state));

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inizializza la struct sockaddr_in del server su cui fare la bind
    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    debug(bind(sockfd, (struct sockaddr *)&server_addr, len));

    // Attesa di un nuovo messaggio da parte di un client
    while (1)
    {
        debug(recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &len));
        handle_message(sockfd, &msg, &state, &client_addr);
    }

    // Liberazione della memoria
    for (int i = 0; i < state.n_clients; i++)
    {
        for (int j = 0; j < state.clients[i].n_messages; j++)
        {
            if (state.clients[i].messages[j] != NULL)
            {
                free(state.clients[i].messages[j]);
            }
        }
        free(state.clients[i].messages);
    }
    free(state.clients);

    // Chiusura del socket
    close(sockfd);
    return 0;
}
