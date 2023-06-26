/**
 * @file client.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Gli studenti del corso di Reti di Calcolatori del DMI – UNICT, hanno deciso,
 * di loro spontanea volontà, di realizzare una chat virtuale,
 * con prestazioni superiori a ChatGPT,
 * al fine di dar vita a 6 linguaggi di programmazione (C, C++, Java, Python, R, Matlab) tramite 6 macchine virtuali.
 * Ognuno dei 6 linguaggi è legato ad una specifica VMClient.
 * Un server, chiamato ServerVMPascal, gestirà la comunicazione tra le varie VMClient.
 * Un operatore umano scriverà nella riga di comando di una VMClient (legata ad un linguaggio X) un numero intero n compreso tra 1 e 6
 * e una stringa di massimo 20 caratteri: queste informazioni che dovranno essere inviate al ServerVM-Pascal,
 * il quale dovrà semplicemente inoltrare la sola stringa agli n linguaggi più vicini al linguaggio X di partenza.
 * Ogni VMClient visualizzerà a schermo la stringa ricevuta.
 * Notare che, il ServerVM-Pascal conterrà al suo interno una matrice (o qualsiasi altra struttura) di distanze,
 * con valori compresi tra 10 e 255, che definiscono la distanza virtuale tra i vari linguaggi di programmazione.
 * La scelta dei protocolli da utilizzare è a discrezione dello studente.
 * @version 0.1
 * @date 2023-06-21
 *
 * @copyright Copyright (c) 2023
 *
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

#define MSG_SIZE 21

/**
 * @brief Il processo figlio si mette in ascolto su una porta prestabilita
 * per ricevere eventuali comunicazioni dal server come risultato del messaggio di un altro client
 * @param client_addr sockaddr su cui mettersi in ascolto
 */
void receiver_process(struct sockaddr_in *client_addr)
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char msg[MSG_SIZE];

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(bind(sockfd, (struct sockaddr *)client_addr, len));
    debug(listen(sockfd, 10));

    int new_sockfd;
    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&server_addr, &len)) > 0)
    {
        if (!fork()) // Figlio
        {
            close(sockfd);
            memset(msg, 0, MSG_SIZE);
            debug(recv(new_sockfd, msg, MSG_SIZE - 1, 0));
            printf("\nMessaggio ricevuto: '%s'\n", msg);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        close(new_sockfd);
    }
    close(sockfd);
}

/**
 * @brief Non appena avviato, il processo padre si registra con il server, inviando la coppia
 * <linguaggio di programmazione> <porta su cui è in ascolto>.
 *
 * Successivamente, si mette in attesa di input da tastiera
 * per poi inviare al server messaggi nel formato
 * <n> <string>
 * @param server_addr sockaddr del server
 * @param language linguaggio di programmazione del client
 * @param port porta su cui il client è in ascolto
 */
void sender_process(struct sockaddr_in *server_addr, const char language[], const char port[])
{
    int sockfd;
    char input[MSG_SIZE + 4], msg[MSG_SIZE];

    memset(msg, 0, MSG_SIZE);
    sprintf(msg, "%s %s", language, port);
    printf("Registrazione al server con linguaggio %s e porta %s\n", language, port);

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(connect(sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in)));
    // Registrazione indicando il linguaggio e porta
    debug(send(sockfd, msg, strlen(msg) + 1, 0));

    while (1)
    {
        printf("Inserire la stringa 'n messaggio' > \n");
        fgets(input, MSG_SIZE + 4, stdin);
        input[strcspn(input, "\n")] = '\0';
        send(sockfd, input, strlen(input) + 1, 0);
    }
}

/**
 * @brief Il client rappresenta fra le 5 macchine associate ad un linguaggio di programmazione:
 * C, C++, Java, Python, R, Matlab
 *
 * Il client deve conoscere l'indirizzo e la porta del server, a cui invierà un messaggio dal formato
 * n string
 * con n: numero [1, 6], string di massimo 20 caratteri
 *
 * Tutti i client rimarranno in ascolto su una porta prestabilita per ricevere eventuali comunicazioni dal server
 * come risultato del messaggio di un altro client
 *
 * @param argc numero di argomenti da riga di comando
 * @param argv lista di stringhe passate da riga di comando
 * @return int codice di uscita
 */
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    if (argc < 5 || atoi(argv[2]) == 0 || atoi(argv[4]) == 0)
    {
        fprintf(stderr, "use: %s <linguaggio di programmazione> <porta client> <ip server> <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin_family = client_addr.sin_family = AF_INET;

    server_addr.sin_port = htons(atoi(argv[4]));
    server_addr.sin_addr.s_addr = inet_addr(argv[3]);

    client_addr.sin_port = htons(atoi(argv[2]));
    client_addr.sin_addr.s_addr = INADDR_ANY;

    int pid = fork();

    if (pid == 0) // figlio
    {
        receiver_process(&client_addr);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0) // padre
    {
        sender_process(&server_addr, argv[1], argv[2]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return 0;
}