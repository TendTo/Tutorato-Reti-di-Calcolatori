#pragma once // Per evitare problemi di inclusioni multiple

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 128

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
    UPDATE = 'u',  // Aggiorna la quantità di vino
    SUCCESS = 's', // Il server ha completato la richiesta con successo
    END = 'e',     // Il server ha terminato l'invio dei messaggi
    FAIL = 'f',    // Il server non è riuscito ad adempiere alla richiesta
    QUIT = 'q',    // Termina il programma
} Action;

typedef struct
{
    long id;                       // Identificativo del messaggio. Usato per modificare o eliminare un messaggio
    char message[MAX_BUFFER_SIZE]; // Messaggio che il client invia al server. Può essere un nuovo messaggio, un messaggio modificato o persino l'username per la registrazione.
                                   // Se è il server il mittente, il messaggio farà parte della lista dei messaggi che il client ha richiesto
    Action action;                 // Azione richiesta che il client fa al server
} Message;
