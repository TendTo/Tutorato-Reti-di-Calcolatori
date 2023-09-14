/**
 * @file azienda.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief L'azienda Rete Enoteca ha deciso di sviluppare una piattaforma per la vendita di vini da differenti aziende.
 * I client-azienda dovranno inviare i prodotti disponibili della propria azienda al server.
 * In particolare, utilizzando un'apposita struttura articolo composta dai seguenti campi (Nome Azienda, id_articolo, Nome Vino, Quantità, Costo), per ogni articolo dovrà essere inviato
 * un pacchetto contenente le seguenti informazioni: Nome Azienda, Nome Vino, Quantità, Costo_
 *
 * Il campo id_articolo (non presente nella lista di cui sopra) dovrà essere aggiunto dal server secondo una qualsiasi euristica definita a piacere
 * Ogni azienda potrà richiedere al sever la propria lista dei vini disponibili, nonché aggiornare la quantità di un singolo articolo specificando l'id_articolo e la Quantità.
 *
 * I client-client avranno la possibili chiedere al server la lista dei prodotti presenti.
 * Enoteca. Quando il client deciderà di acquistare un articolo specifico, dovrà indicare l'id_articolo e la Quantità_.
 * In questo caso, il server dovrà verificare la quantità disponibile e, in caso positivo, decrementa quest'ultima dello specifico articolo.
 * Nel caso in cui la quantità richiesta non sia disponibile, il server invierà al client la quantità disponibile.
 *
 * Lo studente può definire qualsiasi approccio al fine di distinguere i client-client dai client-azienda
 *
 * È consigliato l'uso di un File di testo come memoria condivisa per aggiornare e gestire tutte le operazioni sugli articoli di tutte le aziende.
 * Ad ogni operazione di aggiunta e/o aggiornamento di quantità di un prodotto, verrà aggiornata la struttura articolo e di conseguenza il file di testo.
 * Quindi, ogni volta che un client fa una richiesta, il Server- Enoteca aggiornerà la propria struttura con le informazioni disponibili nel file!
 * @version 0.1
 * @date 2023-09-13
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "definitions.h"

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void init_product(Product *prod, const char company_name[])
{
    memset(prod, 0, sizeof(Product));
    if (company_name != NULL)
        strncpy(prod->company_name, company_name, MAX_NAME_SIZE - 1);
}

void init_message(Message *msg, Action action, const Product *prod)
{
    memset(msg, 0, sizeof(Message));
    msg->type = COMPANY;
    msg->action = action;
    if (prod != NULL)
        msg->prod = *prod;
}

void recv_message(int sockfd, Message *msg)
{
    debug(recv(sockfd, msg, sizeof(Message), 0));
    if (msg->action == SUCCESS)

        printf("Operazione completata con successo!\n");
    else
        printf("Qualcosa e' andato storto con l'ultima richiesta\n");
}

void send_message(int sockfd, Action action, const Product *prod)
{
    Message msg;
    init_message(&msg, action, prod);
    debug(send(sockfd, &msg, sizeof(Message), 0));
}

void prompt(char response[MAX_BUFFER_SIZE], const char prompt[])
{
    memset(response, '\0', MAX_BUFFER_SIZE);
    printf(prompt);
    fgets(response, MAX_BUFFER_SIZE, stdin);
    response[strcspn(response, "\n")] = '\0';
}

void prompt_product(Product *prod, const char company_name[])
{
    char buffer[MAX_BUFFER_SIZE];
    init_product(prod, company_name);

    prompt(buffer, "Inserisci il nome del vino: ");
    strncpy(prod->wine_name, buffer, MAX_NAME_SIZE - 1);
    while (prod->cost == 0)
    {
        prompt(buffer, "Inserisci il costo del vino: ");
        prod->cost = atoi(buffer);
    }
    while (prod->quantity == 0)
    {
        prompt(buffer, "Inserisci la quantità di vini disponibili: ");
        prod->quantity = atoi(buffer);
    }
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in6 server_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char buffer[MAX_BUFFER_SIZE];
    int n_products = 0;
    int product_array[MAX_COMPANY_PRODUCTS];

    if (argc < 4 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <port server> <nome azienda>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[2]));
    inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        Action action;
        printf("Che operazione vuoi effettuare? [i|u|d|e]\n");
        action = getchar();
        while (getchar() != '\n')
            ;
        if (action != INSERT && action != UPDATE && action != DELETE && action != EXIT)
            continue;

        if (action == EXIT) // chiusura client
            break;
        else if (action == INSERT) // Inserimento prodotto
        {
            Product prod;
            prompt_product(&prod, argv[3]);
            send_message(sockfd, INSERT, &prod);

            Message msg;
            recv_message(sockfd, &msg);
            product_array[n_products++] = msg.prod.product_id;
            continue;
        }

        // Un prodotto fra quelli inseriti in precedenza sarà modificato o eliminato
        printf("Che prodotto vuoi modificare? [");
        for (int i = 0; i < n_products; i++)
            printf("%d,", product_array[i]);
        prompt(buffer, "]\n");
        int product_id = atoi(buffer);
        if (product_id == 0)
        {
            printf("Chiusura client");
            break;
        }
        for (int i = 0; i < n_products; i++) // Loop fra tutti i prodotti inseriti dall'azienda
        {
            if (product_id == product_array[i]) // Un elemento della lista corrisponde a ciò che l'utente ha selezionato
            {
                Product prod;
                if (action == UPDATE)
                    prompt_product(&prod, argv[3]); // Permette all'utente d modificare il prodotto
                prod.product_id = product_id;
                send_message(sockfd, action, &prod);

                Message msg;
                recv_message(sockfd, &msg);

                if (action == DELETE) // Se l'utente ha eliminato il prodotto, lo rimuove dalla lista
                {
                    swap(&product_array[i], &product_array[n_products - 1]);
                    n_products--;
                }
                break;
            }
        }
    }

    close(sockfd);

    return 0;
}
