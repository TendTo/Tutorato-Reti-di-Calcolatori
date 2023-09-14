#pragma once // Per evitare problemi di inclusioni multiple

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 128
#define MAX_COMPANY_PRODUCTS 128

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

typedef enum
{
    INSERT = 'i',  // Inserisci un nuovo vino
    DELETE = 'd',  // Elimina un vino
    LIST = 'l',    // Elenca tutti i vini inseriti nel server
    BUY = 'b',     // Acquista un vino
    UPDATE = 'u',  // Aggiorna la quantità di vino
    SUCCESS = 's', // Il server ha completato la richiesta con successo
    FAIL = 'f',    // Il server non è riuscito ad adempiere alla richiesta
    EXIT = 'e',    // Termina il programma
} Action;

typedef enum
{
    COMPANY, // Azienda
    CLIENT,  // Cliente
} ClientType;

typedef struct
{
    unsigned int product_id;          // Id del prodotto. Aggiunto dal server
    char company_name[MAX_NAME_SIZE]; // Nome della compagnia
    char wine_name[MAX_NAME_SIZE];    // Nome del vino
    unsigned int quantity;            // Quantità del prodotto. Può essere aggiornata
    unsigned int cost;                // Costo del prodotto
} Product;

typedef struct
{
    ClientType type; // Tipo di client: azienda o cliente
    Product prod;    // Prodotto
    Action action;   // Azione richiesta al server
} Message;
