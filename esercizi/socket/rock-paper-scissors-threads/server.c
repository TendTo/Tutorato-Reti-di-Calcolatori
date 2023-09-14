/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Si richiede di implementare un programma che simuli il gioco della morra cinese in multiplayer.
 * L'architettura sarà composta da un server che si frappone fra due client, che si sfidano ad una partita.
 * Il server si occuperà di gestire le connessioni e di inviare i messaggi ai client.
 * All'inizio del gioco, i due client si connettono al server, che attende la connessione di entrambi.
 * Non appena entrambi sono pronti, il server invia un messaggio ai client, che indica l'inizio del gioco.
 * A questo punto, i due client inviano al server la propria scelta (sasso, carta o forbice).
 * Non è possibile cambiare la propria scelta una volta inviata.
 * Il server verifica chi ha vinto e invia un messaggio ai client, che indica il vincitore.
 * Il punteggio viene anche aggiornato internamente al server.
 * Alla fine del numero di round stabilito, il server invia un messaggio ai client, che indica la fine del gioco.
 * Le connessioni vengono terminate.
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

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

#define MAX_BUFFER_SIZE 1024
#define MAX_MOVE_SIZE 1
#define CONCLUSION_MSG "La partita si e' conclusa dopo"
#define INVALID_MOVE_MSG "Mossa non valida"

typedef enum
{
    ROCK = 'r',     // Sasso
    PAPER = 'p',    // Carta
    SCISSORS = 's', // Forbice
} Move;

typedef enum
{
    PLAYER_1, // Giocatore 1 vince
    PLAYER_2, // Giocatore 2 vince
    NONE,     // La partita si è conclusa con un pareggio
} Player;

typedef struct
{
    int score[2];              // Punteggio dei giocatori
    int rounds;                // Numero di round da giocare
    Move moves[2];             // Mosse dei giocatori di questo round. Impostati da ogni giocatore
    Player winner;             // Vincitore del round. Impostato dal giudice
    sem_t player_ready_sem[2]; // Coppia di semafori per indicare che i giocatori sono pronti
    sem_t next_round_sem;      // Semaforo per indicare che il prossimo round può iniziare
    int is_running;            // Indica se il gioco è ancora in corso
} Game;

typedef struct
{
    Game *game; // Puntatore alla struttura dati del gioco
    int sockfd;
    Player player;
} ThreadArg;

void init_game(Game *game, int n_rounds)
{
    memset(game, 0, sizeof(Game));
    debug(sem_init(&game->player_ready_sem[PLAYER_1], 0, 0));
    debug(sem_init(&game->player_ready_sem[PLAYER_2], 0, 0));
    debug(sem_init(&game->next_round_sem, 0, 0));
    game->rounds = n_rounds;
}

void clean_game(Game *game)
{
    debug(sem_destroy(&game->player_ready_sem[PLAYER_1]));
    debug(sem_destroy(&game->player_ready_sem[PLAYER_2]));
    debug(sem_destroy(&game->next_round_sem));
}

/**
 * @brief Restituisce una stringa leggibile di un giocatore della partita.
 * @param result un giocatore (o nessun giocatore) sotto forma di enum Player
 * @return char* stringa leggibile del giocatore
 */
char *result_to_string(Player result)
{
    switch (result)
    {
    case PLAYER_1:
        return "Giocatore 1";
    case PLAYER_2:
        return "Giocatore 2";
    case NONE:
        return "Nessuno";
    default:
        return "Errore";
    }
}

/**
 * @brief Restituisce una stringa che rappresenta la mossa.
 * @param move mossa sotto forma di enum Move
 * @return char* stringa che rappresenta la mossa
 */
char *move_to_string(Move move)
{
    switch (move)
    {
    case ROCK:
        return "Sasso";
    case PAPER:
        return "Carta";
    case SCISSORS:
        return "Forbice";
    default:
        return "Errore";
    }
}

Player get_winner(Move moves[2])
{
    if (moves[0] == moves[1])
        return NONE;

    switch (moves[0])
    {
    case ROCK:
        return moves[1] == SCISSORS ? PLAYER_1 : PLAYER_2;
    case PAPER:
        return moves[1] == ROCK ? PLAYER_1 : PLAYER_2;
    case SCISSORS:
        return moves[1] == PAPER ? PLAYER_1 : PLAYER_2;
    default:
        return NONE;
    }
}

void *player_thread(void *args)
{
    ThreadArg *t_args = (ThreadArg *)args;

    Player otherPlayer = 1 - t_args->player; // Se sono PLAYER_1, allora l'altro giocatore è PLAYER_2, e viceversa

    debug(sem_post(&t_args->game->player_ready_sem[t_args->player])); // Il giocatore è pronto
    printf("Giocatore %d pronto\n", t_args->player + 1);
    debug(sem_wait(&t_args->game->next_round_sem)); // Aspetta che l'altro giocatore sia pronto e inizi il round

    // Invia un messaggio al client, che indica l'inizio del gioco
    debug(send(t_args->sockfd, &t_args->game->rounds, sizeof(int), 0));

    while (t_args->game->is_running)
    {
        Move move;

        // Riceve la mossa del giocatore
        debug(recv(t_args->sockfd, &move, sizeof(Move), 0));

        if (move != ROCK && move != PAPER && move != SCISSORS)
        {
            debug(send(t_args->sockfd, INVALID_MOVE_MSG, 17, 0));
            printf("Giocatore %d ha inviato una mossa non valida: %c\n", t_args->player + 1, move);
            continue;
        }
        t_args->game->moves[t_args->player] = move;

        debug(sem_post(&t_args->game->player_ready_sem[t_args->player])); // Il giocatore è pronto

        debug(sem_wait(&t_args->game->next_round_sem)); // Aspetta che il giudice abbia calcolato il vincitore e inizi il prossimo round

        // Invia un messaggio al client, che indica il vincitore
        if (t_args->game->winner == NONE)
            debug(send(t_args->sockfd, "Pareggio", 9, 0));
        else if (t_args->game->winner == t_args->player)
            debug(send(t_args->sockfd, "Hai vinto", 10, 0));
        else
            debug(send(t_args->sockfd, "Hai perso", 10, 0));
    }

    // Invia un messaggio al client, che indica la fine del gioco
    char response[MAX_BUFFER_SIZE];
    sprintf(response, CONCLUSION_MSG " %d round.\n"
                                     "Hai totalizzato un punteggio di %d contro %d del tuo avversario",
            t_args->game->rounds,
            t_args->game->score[t_args->player],
            t_args->game->score[otherPlayer]);
    debug(send(t_args->sockfd, response, strlen(response) + 1, 0));

    close(t_args->sockfd);

    pthread_exit(NULL);
}

void *judge_thread(void *args)
{
    Game *game = (Game *)args;

    sem_wait(&game->player_ready_sem[PLAYER_1]); // Aspetta che il giocatore 1 sia pronto
    sem_wait(&game->player_ready_sem[PLAYER_2]); // Aspetta che il giocatore 2 sia pronto

    printf("Giocatori pronti\n");

    game->is_running = 1;            // La partita è iniziata
    sem_post(&game->next_round_sem); // Inizia il primo round (segnala entrambi i giocatori)
    sem_post(&game->next_round_sem);

    printf("Partita iniziata\n");

    for (int current_round = 0; current_round < game->rounds; current_round++)
    {
        sem_wait(&game->player_ready_sem[PLAYER_1]); // Aspetta che il giocatore 1 sia pronto
        sem_wait(&game->player_ready_sem[PLAYER_2]); // Aspetta che il giocatore 2 sia pronto

        game->winner = get_winner(game->moves); // Calcola il vincitore
        if (game->winner != NONE)
            game->score[game->winner]++; // Aggiorna il punteggio

        printf("Partita %d vinta dal giocatore %d\nPunteggi:\t%d\t%d\n",
               current_round + 1,
               game->winner + 1,
               game->score[PLAYER_1],
               game->score[PLAYER_2]);

        if (current_round == game->rounds - 1)
            game->is_running = 0; // La partita è finita

        sem_post(&game->next_round_sem); // Inizia il prossimo round (segnala entrambi i giocatori)
        sem_post(&game->next_round_sem);
    }

    printf("Partita finita\n");
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int sockfd, player_idx = 0;
    struct sockaddr_in server_addr, client_addr;
    pthread_t players_tid[2], judge_tid;
    socklen_t client_addr_len = sizeof(client_addr);
    Game game;
    ThreadArg t_args[2];

    if (argc < 3 || atoi(argv[1]) == 0 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <port> <n rounds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_game(&game, atoi(argv[2]));

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));
    debug(listen(sockfd, 5));

    debug(pthread_create(&judge_tid, NULL, judge_thread, &game));

    // Accettazione connessione
    int new_sockfd;
    while (player_idx < 2 && (new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        // Dati per ogni thread: socket descriptor e indice del giocatore, nonché un puntatore alla struttura Game
        t_args[player_idx].sockfd = new_sockfd;
        t_args[player_idx].player = player_idx;
        t_args[player_idx].game = &game;

        printf("Giocatore %d connesso\n", player_idx + 1);

        // Creazione del thread del giocatore
        debug(pthread_create(&players_tid[player_idx], NULL, player_thread, &t_args[player_idx]));

        player_idx++;
    }

    printf("Attendiamo che i thread terminino\n");

    // Attende la terminazione di tutti i thread
    pthread_join(players_tid[PLAYER_1], NULL);
    pthread_join(players_tid[PLAYER_2], NULL);
    pthread_join(judge_tid, NULL);

    // Chiusura del socket
    close(sockfd);

    // Rimozione dei semafori
    clean_game(&game);

    return 0;
}