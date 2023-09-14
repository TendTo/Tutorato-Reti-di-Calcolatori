/**
 * @file server.c
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
 * Lo studente può definire qualsiasi approccio al fine di distinguere i client dai client-azienda
 *
 * È consigliato l'uso di un File di testo come memoria condivisa per aggiornare e gestire tutte le operazioni sugli articoli di tutte le aziende.
 * Ad ogni operazione di aggiunta e/o aggiornamento di quantità di un prodotto, verrà aggiornata la struttura articolo e di conseguenza il file di testo.
 * Quindi, ogni volta che un client fa una richiesta, il Server aggiornerà la propria struttura con le informazioni disponibili nel file!
 * @version 0.1
 * @date 2023-09-13
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "definitions.h"

#define DATABASE_FILE "database.txt"
#define LINE_SIZE 512

/**
 * @brief Il client prova ad acquistare un prodotto.
 * Gli unici campi rilevanti del parametro passato sono il product_id, utilizzato per identificare la richiesta,
 * e la quantity, che indica quanti prodotti il client vuole acquistare.
 * Se l'acquisto va a buon fine, il campo quality viene aggiornato sia nel file che nella strcut passata.
 * @param prod prodotto da acquistare. Vengono presi in considerazione solo i campi product_id e quantity
 * @return se l'acquisto è andato a buon fine.
 */
bool buy_product(Product *prod)
{
    bool did_buy = false;
    FILE *database = fopen(DATABASE_FILE, "r");
    FILE *tmp = fopen("tmp.txt", "w");
    if (database == NULL || tmp == NULL)
    {
        perror("buy_product");
        exit(EXIT_FAILURE);
    }

    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, database) != NULL)
    {
        char *product_id_str = strtok(line, ",");
        char *company_name = strtok(NULL, ","); // Nome azienda
        char *wine_name = strtok(NULL, ",");    // Nome vino
        char *quantity_str = strtok(NULL, ",");
        char *cost_str = strtok(NULL, ",");

        int quantity = atoi(quantity_str);
        int product_id = atoi(product_id_str);

        if (product_id != prod->product_id) // Se non siamo alla riga di product
        {
            fprintf(tmp, "%d,%s,%s,%s,%s",
                    product_id,
                    company_name,
                    wine_name,
                    quantity_str,
                    cost_str);
        }
        else if (quantity >= prod->quantity) // Se il client sta comprando il vino, aggiorna la quantità
        {
            fprintf(tmp, "%d,%s,%s,%d,%s",
                    product_id,
                    company_name,
                    wine_name,
                    quantity - prod->quantity,
                    cost_str);
            prod->quantity = quantity - prod->quantity;
            did_buy = true;
        }
        else // Non c'è abbastanza vino
        {
            fprintf(tmp, "%d,%s,%s,%s,%s",
                    product_id,
                    company_name,
                    wine_name,
                    quantity_str,
                    cost_str);
            fprintf(stderr, "Not enough quantity: requested %d, available %d\n", prod->quantity, quantity);
            prod->quantity = quantity;
        }
    }
    fclose(database);
    fclose(tmp);
    remove(DATABASE_FILE);
    rename("tmp.txt", DATABASE_FILE);
    return did_buy;
}

/**
 * @brief Aggiorna il prodotto nel database con quello nuovo.
 * La ricerca avviene limitandosi a confrontare l'id con quelli già presenti.
 * Se il prodotto nuovo è NULL, il prodotto trovato viene semplicemente rimosso.
 * @example update_product(prod, new_prod) // se prod è presente nel file, viene sostituito da new_prod
 * @example update_product(prod, NULL) // se prod è presente nel file, viene rimosso
 * @param prod prodotto da aggiornare o rimuovere
 * @param new_prod prodotto con cui sostituire quello vecchio. Se NULL, l'originale viene eliminato
 */
void update_product(Product *prod, Product *new_prod)
{
    FILE *database = fopen(DATABASE_FILE, "r");
    FILE *tmp = fopen("tmp.txt", "w");
    if (database == NULL || tmp == NULL)
    {
        perror("remove_from_file");
        exit(EXIT_FAILURE);
    }

    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, database) != NULL)
    {
        char *product_id_str = strtok(line, ",");
        int product_id = atoi(product_id_str);
        product_id_str[strlen(product_id_str)] = ','; // Ripristina la virgola nella riga
        if (product_id != prod->product_id)           // Copia tutto ciò che non è il prodotto da aggiornare/rimuovere
            fputs(line, tmp);
        else if (new_prod != NULL) // Se siamo alla riga di product e new_product è non nullo, inserisce quest'ultimo al posto dell'originale
            fprintf(tmp, "%d,%s,%s,%d,%d\n",
                    new_prod->product_id,
                    new_prod->company_name,
                    new_prod->wine_name,
                    new_prod->quantity,
                    new_prod->cost);
    }
    fclose(database);
    fclose(tmp);
    remove(DATABASE_FILE);
    rename("tmp.txt", DATABASE_FILE);
}

/**
 * @brief Ogni volta che un nuovo client si registra, verrà aggiunta una riga al file di database
 * nel formato
 * <product_id>,<company_name>,<wine_name>,<quantity>,<cost>
 * @param prod prodotto da inserire nel file
 */
void save_product(Product *prod)
{
    FILE *database = fopen(DATABASE_FILE, "a");
    if (database == NULL)
    {
        perror("database");
        exit(EXIT_FAILURE);
    }
    fprintf(database, "%d,%s,%s,%d,%d\n",
            prod->product_id,
            prod->company_name,
            prod->wine_name,
            prod->quantity,
            prod->cost);
    fclose(database);
}

void get_product_ids(int product_ids[MAX_COMPANY_PRODUCTS])
{
    FILE *database = fopen(DATABASE_FILE, "r");
    if (database == NULL)
    {
        perror("database");
        exit(EXIT_FAILURE);
    }

    char line[LINE_SIZE];
    int idx = 0;
    while (fgets(line, LINE_SIZE, database) != NULL)
    {
        char *product_id_str = strtok(line, ",");
        int product_id = atoi(product_id_str);
        product_ids[idx++] = product_id;
    }
    fclose(database);
}

void print_product(const Product *prod)
{
    printf("Product: {\ncompany_name: %s,\nwine_name: %s,\nproduct_id: %d,\nquantity: %d,\ncost: %d}\n",
           prod->company_name,
           prod->wine_name,
           prod->product_id,
           prod->quantity,
           prod->cost);
}

bool handle_client(int sockfd, Message *msg)
{
    switch (msg->action)
    {
    case LIST:
        int product_ids[MAX_COMPANY_PRODUCTS]; // Lista degli id dei prodotti
        memset(product_ids, 0, sizeof(product_ids));
        get_product_ids(product_ids);
        debug(send(sockfd, &product_ids, sizeof(product_ids), 0));
        return true;
    case BUY:
        return buy_product(&msg->prod);
    default:
        fprintf(stderr, "Invalid action: '%c'\nNothing to do\n", msg->action);
        return false;
    }
}

bool handle_company(Message *msg)
{
    switch (msg->action)
    {
    case INSERT:
        msg->prod.product_id = time(NULL); // Tanto abbiamo meno di un'aggiunta al secondo, giusto?
        save_product(&msg->prod);
        return true;
    case DELETE:
        update_product(&msg->prod, NULL);
        return true;
    case UPDATE:
        update_product(&msg->prod, &msg->prod); // Tanto l'id del prodotto è lo stesso
        return true;
    default:
        fprintf(stderr, "Invalid action: '%c'\nNothing to do\n", msg->action);
        return false;
    }
}

/**
 * @brief Gestisce i messaggi ricevuti dalle aziende o dai client.
 * @param sockfd socket in ascolto
 */
void handle_message(int sockfd)
{
    bool res;
    Message msg;
    memset(&msg, sizeof(Message), 0);

    while (1)
    {
        debug(recv(sockfd, &msg, sizeof(Message), 0));
        switch (msg.type)
        {
        case COMPANY:
            res = handle_company(&msg);
            break;
        case CLIENT:
            res = handle_client(sockfd, &msg);
            break;
        default:
            fprintf(stderr, "Invalid client type\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        msg.action = res ? SUCCESS : FAIL;
        debug(send(sockfd, &msg, sizeof(Message), 0));
    }
    close(sockfd);
}

int main(int argc, char const *argv[])
{
    int sockfd, new_sockfd;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in6);

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Crea e resetta il file database
    FILE *database = fopen(DATABASE_FILE, "w");
    if (database == NULL)
    {
        perror("database");
        exit(EXIT_FAILURE);
    }
    fclose(database);

    // Creazione del socket
    debug(sockfd = socket(AF_INET6, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin6_addr = in6addr_any;          // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 10));

    // Attesa della connessione dei due giocatori
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0)
    {
        pid_t pid = fork();
        if (pid == 0) // Processo figlio
        {
            close(sockfd);
            handle_message(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        else if (pid > 0) // Processo padre
        {
            close(new_sockfd);
        }
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Chiusura del socket
    close(sockfd);
}
