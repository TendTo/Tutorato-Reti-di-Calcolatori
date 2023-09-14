/**
 * @file client.c
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

void print_array(int array[])
{
    printf("[");
    for (int *iterator = array; *iterator != 0; iterator++)
    {
        printf("%d, ", *iterator);
    }
    printf("]\n");
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
    msg->type = CLIENT;
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
    int product_array[MAX_COMPANY_PRODUCTS];
    memset(product_array, 0, sizeof(product_array));

    if (argc < 3 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <ip server> <port server>\n", argv[0]);
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
        printf("Che operazione vuoi effettuare? [l|b|e]\n");
        action = getchar();
        while (getchar() != '\n')
            ;
        if (action != LIST && action != BUY && action != EXIT)
            continue;

        if (action == EXIT) // chiusura client
            break;
        else if (action == LIST) // Mostra la lista dei prodotti presenti nel server
        {
            Message msg;
            send_message(sockfd, LIST, NULL);
            memset(product_array, 0, sizeof(product_array));
            debug(recv(sockfd, &product_array, sizeof(product_array), 0));
            print_array(product_array);
            recv_message(sockfd, &msg);
            continue;
        }
        else if (action == BUY)
        {
            print_array(product_array);
            prompt(buffer, "Che prodotto vuoi acquistare?\n");
            for (int *prod_ptr = product_array; *prod_ptr > 0; prod_ptr++)
            {
                if (*prod_ptr == atoi(buffer))
                {
                    Product prod;
                    init_product(&prod, NULL);
                    prod.product_id = atoi(buffer);
                    do
                    {
                        prompt(buffer, "Quanti prodotti vuoi acquistare?\n");
                    } while (atoi(buffer) == 0);
                    prod.quantity = atoi(buffer);
                    send_message(sockfd, BUY, &prod);

                    Message msg;
                    recv_message(sockfd, &msg);
                    if (msg.action == FAIL)
                        printf("Non ci sono abbastanza prodotti disponibili\nNe rimangono solo %d\n", msg.prod.quantity);
                    break;
                }
            }
            continue;
        }
    }

    close(sockfd);

    return 0;
}
