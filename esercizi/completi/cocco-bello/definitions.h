#pragma once // Per evitare problemi di inclusioni multiple

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 128
#define NUM_PRODUCTS 4
#define MAX_NUM_USERS 10

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
    AUTHENTICATE = 'a',
    ORDER = 'o',
    LIST = 'l',
    QUIT = 'q',
} Action;

typedef struct
{
    Action action;
    char data[MAX_BUFFER_SIZE];
} Request;

typedef struct
{
    unsigned int product_id;          // Id del prodotto. Usato per fare richieste al server
    char product_name[MAX_NAME_SIZE]; // Nome del prodotto
    unsigned int price;               // Costo del prodotto
    unsigned int quantity;            // Quantità disponibile
} Product;

typedef struct
{
    int sockdf;
    char username[MAX_NAME_SIZE];
} User;
