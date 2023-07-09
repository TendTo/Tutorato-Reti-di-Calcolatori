/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Realizzare un programma scritto in c (o c++) che simuli una partita a "Tris" fra i due Client, con il Server in mezzo a fare da arbitro.
 * Si assuma che il numero di giocatori sia esattamente 2 e che tutti rispettino il protocollo descritto.
 * Appena avviato, ogni Client si connette al Server per registrarsi, fornendo anche le informazioni che il Server userà per comunicare durante la partita (ip, porta).
 * Quando entrambi i Client si sono registrati, il Server avvia la partita.
 * Inizia quindi ad interpellare a turno i due giocatori, chiedendo dove porre il loro simbolo sulla griglia.
 * Dopo ogni mossa, il Server risponde inviando la griglia aggiornata.
 * Quando un giocatore ha vinto, il Server invia un messaggio di vittoria al vincitore e di sconfitta al perdente, quindi termina la partita.
 * @version 0.1
 * @date 2023-07-09
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

#define GRID_SIZE 9
#define N_PLAYERS 2
#define MAX_BUFFER_SIZE 1024
#define MAX_MOVE_SIZE 1
#define CONCLUSION_MSG "La partita si e' conclusa dopo"
#define INVALID_MOVE_MSG "Mossa non valida, turno da rifare"
#define REQ_MOVE "Fai la tua mossa"

typedef enum
{
    PLAYER_1_TURN, // È il turno del giocatore 1
    PLAYER_2_TURN, // È il turno del giocatore 2
    END,           // La partita è finita
} PlayerTurn;

typedef enum
{
    NONE,     // La partita è ancora in corso
    PLAYER_1, // Ha vinto il giocatore 1
    PLAYER_2, // Ha vinto il giocatore 2
    DRAW,     // La partita è finita in pareggio
} Winner;

typedef struct
{
    int sockfd[N_PLAYERS]; // Socket dei due giocatori
    int grid[GRID_SIZE];   // Griglia di gioco
} Game;

/**
 * @brief Ottiene la griglia di gioco come stringa.
 * La stringa è formattata come una matrice 3x3, con le celle separate da "|".
 * Va liberata con free().
 * @param game struttura che contiene le informazioni della partita
 * @return stringa che rappresenta la griglia di gioco
 */
char *grid_to_str(Game *game)
{
    char grid_str[MAX_BUFFER_SIZE];
    memset(grid_str, 0, MAX_BUFFER_SIZE);

    for (int i = 0; i < GRID_SIZE; i++)
    {
        strcat(grid_str, "|");
        switch (game->grid[i])
        {
        case PLAYER_1:
            strcat(grid_str, " X ");
            break;
        case PLAYER_2:
            strcat(grid_str, " O ");
            break;
        case NONE:
            strcat(grid_str, "   ");
            break;
        default:
            fprintf(stderr, "Errore: valore non valido nella griglia\n");
            strcat(grid_str, " E ");
            break;
        }

        if ((i + 1) % 3 == 0)
        {
            strcat(grid_str, "|\n");
        }
    }
    return strdup(grid_str);
}

char *result_to_str(Winner winner)
{
    switch (winner)
    {
    case PLAYER_1:
        return "Il giocatore 1";
    case PLAYER_2:
        return "Il giocatore 2";
    case DRAW:
        return "Nessuno";
    default:
        fprintf(stderr, "Errore: valore non valido nella griglia\n");
        return "ERROR";
    }
}

/**
 * @brief Controlla l'intera griglia per verificare lo stato della partita.
 * Se nessuno ha vinto e la griglia è piena, ritorna DRAW.
 * Altrimenti ritorna il giocatore che ha vinto.
 * Se la partita non è ancora finita, ritorna NONE.
 * @param game struttura che contiene le informazioni della partita
 * @return il giocatore che ha vinto la partita, o NONE se nessuno ha ancora vinto o DRAW se la partita è finita in pareggio
 */
Winner is_game_ended(Game *game)
{
    // Controlla ogni riga
    for (int i = 0; i < GRID_SIZE; i += 3)
        if (game->grid[i] != NONE && game->grid[i] == game->grid[i + 1] && game->grid[i] == game->grid[i + 2])
            return game->grid[i];

    // Controlla ogni colonna
    for (int i = 0; i < 3; i++)
        if (game->grid[i] != NONE && game->grid[i] == game->grid[i + 3] && game->grid[i] == game->grid[i + 6])
            return game->grid[i];

    // Controlla le diagonali
    if (game->grid[0] != NONE && game->grid[0] == game->grid[4] && game->grid[0] == game->grid[8])
        return game->grid[0];
    if (game->grid[2] != NONE && game->grid[2] == game->grid[4] && game->grid[2] == game->grid[6])
        return game->grid[2];

    // Controlla se la griglia è piena
    for (int i = 0; i < GRID_SIZE; i++)
        if (game->grid[i] == NONE)
            return NONE;

    // Se nessuno ha vinto e la griglia è piena, è pareggio
    return DRAW;
}

void judge(Game *game)
{
    char result[MAX_BUFFER_SIZE], *grid_str;
    printf("Inizio della partita\n");

    PlayerTurn player_turn = PLAYER_1_TURN;
    while (player_turn != END) // Ogni turno
    {
        printf("Turno del giocatore %d\n", player_turn + 1);
        // Controlla se la partita è finita
        Winner winner = is_game_ended(game);
        char *grid_str = grid_to_str(game);

        switch (winner)
        {
        case NONE:
            // Se ancora la partita è in corso, invia la griglia di gioco al prossimo giocatore
            sprintf(result, "Stato della partita:\n%s", grid_str);
            debug(send(game->sockfd[player_turn], result, sizeof(result), 0));
            break;
        case PLAYER_1:
        case PLAYER_2:
        case DRAW:
            // Se qualcuno ha vinto o c'è stato un pareggio, termina la partita
            sprintf(result, "%s ha vinto la partita\n%s", result_to_str(winner), grid_str);
            for (int i = 0; i < N_PLAYERS; i++)
                debug(send(game->sockfd[i], result, sizeof(result), 0));
            break;
        default:
            break;
        }
        free(grid_str);

        int position;
        printf("Attesa della mossa del giocatore %d\n", player_turn + 1);

        // Ricevi la mossa del giocatore corrente
        debug(recv(game->sockfd[player_turn], &position, sizeof(int), 0));
        // Decrementa la posizione per adattarla all'array
        position--;

        printf("Il giocatore %d ha scelto la posizione %d\n", player_turn + 1, position);

        if (position < 0 || position >= GRID_SIZE || game->grid[position] != NONE)
        {
            printf("Mossa non valida\n");
            continue;
        }

        // Aggiorna la griglia di gioco
        game->grid[position] = player_turn + 1;

        player_turn = winner == NONE ? (player_turn + 1) % 2 : END;
    }
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in6);
    Game game;
    memset(&game, 0, sizeof(Game));

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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
    for (int i = 0; i < N_PLAYERS; i++)
    {
        printf("Attesa di connessione\n");
        debug(game.sockfd[i] = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len));
        printf("Connessione stabilita con il giocatore %d\n", i + 1);
    }

    // Inizio della partita
    judge(&game);

    // Chiusura del socket
    close(sockfd);

    return 0;
}