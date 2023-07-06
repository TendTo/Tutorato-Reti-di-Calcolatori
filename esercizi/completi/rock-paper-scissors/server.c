/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare un programma scritto in c (o c++) che simuli una partita a "Carta-Sasso-Forbice" fra i due Client, con il Server in mezzo a fare da arbitro.
 * Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.
 * Appena avviato, ogni Client si connette al Server per registrarsi, fornendo anche le informazioni che il Server userà per comunicare durante la partita (nome, ip, porta).
 * Quando entrambi i Client si sono registrati, il Server avvia la partita, impostando il numero di vite di entrambi i giocatori ad un valore stabilito.
 * Inizia quindi ad interpellare a turno i due giocatori, chiedendo la loro mossa.
 * Quando entrambi hanno giocato, il Server comunica il risultato della partita a entrambi i giocatori.
 * Se non si è verificato un pareggio, toglie una vita a quello che ha perso e ricomincia il ciclo.
 * Quando un giocatore rimane senza vite, il Server comunica il vincitore ad entrambi e termina la partita.
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright (c) 2023
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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

#define N_PLAYERS 2
#define MAX_BUFFER_SIZE 1024
#define MAX_MOVE_SIZE 1
#define CONCLUSION_MSG "La partita si e' conclusa dopo"
#define INVALID_MOVE_MSG "Mossa non valida, turno da rifare"
#define REQ_MOVE "Fai la tua mossa"

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
    int sockfd;                        // Socket del server
    struct sockaddr_in player_addr[2]; // Indirizzo dei due giocatori
    socklen_t player_addr_len[2];      // Lunghezza dell'indirizzo dei due giocatori
    int lives[2];                      // Vite rimanenti dei due giocatori
    int rounds;                        // Numero di round giocati
    Move moves[2];                     // Mosse dei giocatori di questo round. Impostati da ogni giocatore
    Player winner;                     // Vincitore del round. Impostato dal server
} Game;

void init_game(Game *game, int sockfd, int lives)
{
    memset(game, 0, sizeof(Game));
    game->sockfd = sockfd;
    game->rounds = 0;
    game->lives[PLAYER_1] = lives;
    game->lives[PLAYER_2] = lives;
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

/**
 * @brief Restituisce il vincitore del round.
 * Valuta le mosse dei giocatori e restituisce il vincitore.
 * @param moves mosse dei giocatori
 * @return giocatore vincitore del round. Se la partita è conclusa con un pareggio, restituisce NONE
 */
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

void judge(Game *game)
{
    printf("Inizio della partita\n");

    while (1) // Ogni turno
    {
        int invalid_turn = 0;

        printf("Attesa della mossa dei giocatori\n");

        for (int i = 0; i < N_PLAYERS; i++)
        {
            // Richiedi al giocatore corrente di fare la mossa
            debug(sendto(game->sockfd, REQ_MOVE, sizeof(REQ_MOVE), 0,
                         (struct sockaddr *)&game->player_addr[i], game->player_addr_len[i]));
            // Ricevi la mossa del giocatore corrente
            debug(recvfrom(game->sockfd, game->moves + i, sizeof(Move), 0, NULL, NULL));
        }

        printf("Round %d\nMossa giocatore 1: %s\nMossa giocatore 2: %s\n",
               game->rounds + 1, move_to_string(game->moves[0]), move_to_string(game->moves[1]));

        for (int i = 0; i < N_PLAYERS; i++)
        {
            // Mossa non valida
            if (game->moves[i] != 'r' && game->moves[i] != 'p' && game->moves[i] != 's')
                invalid_turn = 1;
            // Il turno è stato invalidato, skippiamo il turno e mandiamo un messaggio di errore
            if (invalid_turn)
                debug(sendto(game->sockfd, INVALID_MOVE_MSG, sizeof(INVALID_MOVE_MSG), 0,
                             (struct sockaddr *)&game->player_addr[i], game->player_addr_len[i]));
        }
        if (invalid_turn)
            continue;

        game->rounds++; // Aggiorna il numero di round giocati
        game->winner = get_winner(game->moves);
        if (game->winner != NONE)
            game->lives[game->winner]--; // Aggiorna le vite del vincitore

        if (game->lives[0] == 0 || game->lives[1] == 0)
            break;

        char result[MAX_BUFFER_SIZE];
        sprintf(result, "%s ha vinto il round %d.\nIl giocatore 1 ha %d vite, il giocatore 2 ha %d vite",
                result_to_string(game->winner), game->rounds,
                game->lives[PLAYER_1], game->lives[PLAYER_2]);

        // Se la partita è ancora in corso, invia il risultato del round ai giocatori
        for (int i = 0; i < N_PLAYERS; i++)
            debug(sendto(game->sockfd, result, sizeof(result) + 1, 0,
                         (struct sockaddr *)&game->player_addr[i], game->player_addr_len[i]));
    }

    printf("Fine della partita\n");
    char result[MAX_BUFFER_SIZE];
    sprintf(result, "%s ha vinto il round %d. E con quest'ultima mossa, %s ha vinto la partita!",
            result_to_string(game->winner),
            game->rounds,
            result_to_string(game->winner));

    // Invia il risultato della partita ai giocatori
    for (int i = 0; i < N_PLAYERS; i++)
        debug(sendto(game->sockfd, result, sizeof(result) + 1, 0,
                     (struct sockaddr *)&game->player_addr[i], game->player_addr_len[i]));
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    Game game;

    if (argc < 3 || atoi(argv[1]) == 0 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "Usage: %s <port> <lives per player>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione del socket
    debug(sockfd = socket(AF_INET, SOCK_DGRAM, 0));
    // Permette di riutilizzare la porta anche se già in uso
    debug(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    // Inizializzazione della struttura server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1])); // Porta del server
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Accetta connessioni da qualsiasi indirizzo

    // Binding
    debug(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    // Inizializza la struct Game con tutti i valori attesi
    init_game(&game, sockfd, atoi(argv[2]));

    // Attesa della connessione dei due giocatori
    for (int i = 0; i < N_PLAYERS; i++)
    {
        // Non mi aspetto di ricevere dati, solo una connessione per inizializzare l'indirizzo del giocatore
        debug(recvfrom(sockfd, NULL, 0, 0, (struct sockaddr *)&client_addr, &client_addr_len));
        game.player_addr[i] = client_addr;
        game.player_addr_len[i] = client_addr_len;
        printf("Giocatore %d connesso\nIp: %s, port %d\n", i + 1, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    // Inizio della partita
    judge(&game);

    // Chiusura del socket
    close(sockfd);

    return 0;
}