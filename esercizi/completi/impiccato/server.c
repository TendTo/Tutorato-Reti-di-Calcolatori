/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Due macchine client si sfideranno tra di loro a fine di indovinare la parola segreta presente in una macchina server.
 * Ogni macchina client avrà a disposizione 3 vite. Il conteggio delle vite sarà tenuto dal server.
 * Ad ogni turno:
 *
 * - Il server prima invierà al client di turno lo stato del gioco: lettere indovinate, lettere mancanti e struttura della parola
 * - Ogni client ad ogni turno avrà la possibilità solo di scrivere da riga di comando o una lettere o dare la risposta completa.
 *   In caso di errore, perderà una vita.
 *   Il server notificherà allo specifico client l'esito del turno.
 * - Se il giocatore di turno perde tutte le vite, dovrà terminare l'esecuzione del programma.
 *
 * Il server dovrà memorizzare in una struct client il nome del giocatore, l'indirizzo IP e la porta del client.
 * Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.
 * Il server sceglierà randomicamente una stringa da utilizzare per il gioco dell'impiccato tra quelle presenti in un array bidimensionale.
 * Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).
 * Progettare ed implementare **L'impiccato!** tramite socket UDP, considerando l'architettura vista prima.
 * @version 0.1
 * @date 2023-07-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 32
#define MAX_PLAYERS 2

typedef struct
{
    char name[MAX_NAME_SIZE]; // Nome del giocatore
    in_addr_t ip;             // Indirizzo IP del giocatore, preso direttamente dalla struct sockaddr_in.sin_addr.s_addr
    int port;                 // Porta del giocatore, presa direttamente dalla struct sockaddr_in.sin_port
    int lives;                // Vite del giocatore, parte da 3
} Player;

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    Player players[MAX_PLAYERS]; // Array che contiene i 2 giocatori
    char buffer[MAX_BUFFER_SIZE];
    char *word_list[] = {"ciao", "fest", "casa", "balcone", "bottiglia", "albero", "rum", "computer"}; // Lista di parole da indovinare

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Seleziona una parola random dalla lista
    srand(time(NULL));
    char *word_to_guess = word_list[rand() % (sizeof(word_list) / sizeof(char *))];
    // Crea una stringa con la stessa lunghezza della parola da indovinare, ma con tutti i caratteri '_'
    char *word_state = strdup(word_to_guess);
    for (int i = 0; i < strlen(word_state); i++)
        word_state[i] = '_';

    printf("Parola da indovinare: '%s' -> '%s'\n", word_to_guess, word_state);

    // Inizializza la struct sockaddr_in del server su cui fare la bind
    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    perror("socket");

    bind(sockfd, (struct sockaddr *)&server_addr, len);
    perror("bind");

    // Attende la connessione dai 2 giocatori
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        printf("Attesa giocatore %d\n", i);

        // Salva tutti i dati del giocatore nella struct
        memset(&players[i], 0, sizeof(Player));
        recvfrom(sockfd, &players[i].name, MAX_NAME_SIZE, 0, (struct sockaddr *)&client_addr, &len);
        players[i].lives = 3;
        players[i].ip = client_addr.sin_addr.s_addr;
        players[i].port = ntohs(client_addr.sin_port);

        printf("Registrazione giocatore %s, porta %d e ip %s avvenuta con successo!\n", players[i].name, players[i].port, inet_ntoa(client_addr.sin_addr));
    }

    int current_player = 0;
    while (players[0].lives > 0 || players[1].lives > 0) // Finché almeno un giocatore ha vite
    {
        if (players[current_player].lives == 0) // Se il giocatore di turno non ha più vite, passa il turno immediatamente
        {
            current_player = 1 - current_player;
            continue;
        }

        // Inizializza la struct sockaddr_in del giocatore corrente per inviargli i messaggi
        client_addr.sin_addr.s_addr = players[current_player].ip;
        client_addr.sin_port = htons(players[current_player].port);

        // Invia al giocatore corrente la stringa con la parola da indovinare e le vite dei giocatori
        // Questo sblocca il client e gli permette di inviare il suo tentativo
        memset(buffer, 0, MAX_BUFFER_SIZE);
        snprintf(buffer, MAX_BUFFER_SIZE, "Parola corrente: %s | Vite %s: %d - %s: %d",
                 word_state, players[0].name, players[0].lives, players[1].name, players[1].lives);
        printf("Sto inviando al player %d la stringa:\n%s\n", current_player, buffer);
        sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client_addr, len);

        // Riceve il tentativo del giocatore corrente
        memset(buffer, 0, MAX_BUFFER_SIZE);
        recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &len);

        printf("Il client ha inviato come tentativo '%s'\n", buffer);

        int should_lose_life = 1;
        if (strlen(buffer) == 1) // Il client sta provando ad indovinare una singola lettera
        {
            char letter = buffer[0];
            for (int i = 0; i < strlen(word_to_guess); i++)
            {
                if (word_to_guess[i] == letter)
                {
                    should_lose_life = 0;
                    word_state[i] = letter;
                }
            }
        }
        else if (strcmp(word_to_guess, buffer) == 0) // Il client sta provando ad indovinare la parola
            strcpy(word_state, word_to_guess);

        if (strchr(word_state, '_') == NULL) // Se non rimangono più '_' nella parola da indovinare, il giocatore ha vinto
        {
            // Invia il messaggio di vittoria e termina il programma
            memset(buffer, 0, MAX_BUFFER_SIZE);
            snprintf(buffer, MAX_BUFFER_SIZE, "Hai vinto, partita finita.\nLa parola era: %s | Vite %s: %d - %s: %d",
                     word_to_guess, players[0].name, players[0].lives, players[1].name, players[1].lives);
            printf("Sto inviando al player %d la stringa:\n%s\n", current_player, buffer);
            sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client_addr, len);
            perror("sendto win");

            // L'altro giocatore ha perso, invia il messaggio di sconfitta per far terminare il client
            client_addr.sin_addr.s_addr = players[1 - current_player].ip;
            client_addr.sin_port = htons(players[1 - current_player].port);
            memset(buffer, 0, MAX_BUFFER_SIZE);
            snprintf(buffer, MAX_BUFFER_SIZE, "Game over, partita finita.\nUltimo stato: %s | Vite %s: %d - %s: %d",
                     word_state, players[0].name, players[0].lives, players[1].name, players[1].lives);
            printf("Sto inviando al player %d la stringa:\n%s\n", current_player, buffer);
            sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client_addr, len);
            perror("sendto lose other player");
            break;
        }

        if (should_lose_life) // Se il giocatore non ha indovinato, perde una vita
            players[current_player].lives--;

        if (players[current_player].lives == 0) // Se il giocatore ha perso tutte le vite, invia il messaggio di sconfitta
        {
            memset(buffer, 0, MAX_BUFFER_SIZE);
            snprintf(buffer, MAX_BUFFER_SIZE, "Game over, partita finita.\nUltimo stato: %s | Vite %s: %d - %s: %d",
                     word_state, players[0].name, players[0].lives, players[1].name, players[1].lives);
            printf("Sto inviando al player %d la stringa:\n%s\n", current_player, buffer);
            sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&client_addr, len);
            perror("sendto lose");
        }

        // Passa il turno all'altro giocatore
        current_player = 1 - current_player;
    }

    free(word_state); // Libera la memoria allocata per la parola da indovinare
    close(sockfd);
    return 0;
}