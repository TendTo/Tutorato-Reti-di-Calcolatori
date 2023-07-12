/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Due macchine client si sfideranno tra di loro a fine di vincere una partita di Forza 4, con una griglia 6x7.
 * Il server dovrà gestire la partita, controllare l'eventuale vincitore e notificare i client sullo stato della partita.
 * Ad ogni turno:
 *
 * - Il server prima invierà al client di turno lo stato del gioco, ovvero la griglia di gioco con le pedine già posizionate.
 * - Ogni client ad ogni turno avrà la possibilità solo di indicare la colonna in cui posizionare la pedina (1 - 7). Se la mossa non è valida, il server dovrà notificare il client e chiedere di ripetere la mossa.
 * - Non appena il giocatore avrà posizionato la pedina, il server dovrà controllare se il giocatore ha vinto o meno. In caso di vittoria, il server dovrà notificare il client e terminare la partita.
 * - Se non è più possibile fare mosse, la partita finisce in pareggio.
 * - Si vince quando si allineano 4 pedine dello stesso colore in orizzontale, verticale o diagonale.
 * - I client dovranno attendere il proprio turno per giocare, probabilmente bloccati da una `recvfrom()`.
 * - Ad ogni mossa, il client indica solo la colonna dove inserire la pedina, che "cadrà" riempendo la prima casella vuota della colonna.
 * - Non è possibile inserire pedine in una colonna già piena.
 *
 * Il server dovrà memorizzare in una struct client il nome del giocatore e tutte le informazioni necessarie a comunicare con il client successivamente.
 * Notare che, massimo possono giocare solo 2 client a partita e il gioco inizierà solo quando il server avrà registrato (struct client) i due giocatori.
 * I client seguono il protocollo onestamente.
 * Il server sceglierà il client che inizierà la partita con modalità a scelta libera dello studente (random, il primo che si è registrato, ecc...).
 *
 * Progettare ed implementare Forza 4 tramite TCP o UDP, considerando l'architettura vista prima.
 * @version 0.1
 * @date 2023-07-12
 *
 * @copyright Copyright (c) 2023
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

typedef enum
{
    RED,    // Pedina rossa del giocatore 1
    YELLOW, // Pedina gialla del giocatore 2
    EMPTY,  // Casella vuota
} Color;

typedef struct
{
    char name[MAX_NAME_SIZE]; // Nome del giocatore
    int sockfd;               // Socket associato al giocatore
} Player;

typedef struct
{
    int grid[6][7];              // Griglia di gioco
    int turn;                    // Giocatore di turno
    int winner;                  // Vincitore
    Player players[MAX_PLAYERS]; // Array di giocatori
} Game;

void init_game(Game *game)
{
    game->turn = 0;
    game->winner = -1;
    memset(game->players, 0, sizeof(game->players));
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 7; j++)
            game->grid[i][j] = EMPTY;
}

void print_grid(char buffer[MAX_BUFFER_SIZE], Game *game)
{
    memset(buffer, 0, MAX_BUFFER_SIZE);
    for (int i = 5; i >= 0; i--)
    {
        strcat(buffer, "|");
        for (int j = 0; j < 7; j++)
        {
            switch (game->grid[i][j])
            {
            case RED:
                strcat(buffer, " R |");
                break;
            case YELLOW:
                strcat(buffer, " Y |");
                break;
            case EMPTY:
                strcat(buffer, "   |");
                break;
            default:
                strcat(buffer, " ? |");
                break;
            }
        }
        strcat(buffer, "\n");
    }
    strcat(buffer, "  1   2   3   4   5   6   7\n");
}

void get_winner(Game *game, int x, int y)
{
    printf("Controllo orizzontale...\n");
    for (int offset = -3; offset <= 0; offset++)
    {
        int start_x = x + offset;
        if (start_x < 0 || start_x + 3 >= 7)
            continue;
        for (int i = 0; i < 4; i++)
        {
            if (game->grid[y][start_x + i] == game->turn && game->grid[y][start_x + i + 1] == game->turn && game->grid[y][start_x + i + 2] == game->turn && game->grid[y][start_x + i + 3] == game->turn)
            {
                printf("Vittoria orizzontale!\n");
                game->winner = game->turn;
                return;
            }
        }
    }

    printf("Controllo verticale...\n");
    for (int offset = -3; offset <= 0; offset++)
    {
        int start_y = y + offset;
        if (start_y < 0 || start_y + 3 >= 6)
            continue;
        for (int i = 0; i < 4; i++)
        {
            if (game->grid[start_y + i][x] == game->turn && game->grid[start_y + i + 1][x] == game->turn && game->grid[start_y + i + 2][x] == game->turn && game->grid[start_y + i + 3][x] == game->turn)
            {
                printf("Vittoria verticale!\n");
                game->winner = game->turn;
                return;
            }
        }
    }

    printf("Controllo diagonale...\n");
    for (int offset = -3; offset <= 0; offset++)
    {
        int start_y = y + offset;
        int start_x = x + offset;
        if (start_y < 0 || start_y + 3 >= 6 || start_x < 0 || start_x + 3 >= 7)
            continue;
        for (int i = 0; i < 4; i++)
        {
            if (game->grid[start_y + i][start_x + i] == game->turn && game->grid[start_y + i + 1][start_x + i + 1] == game->turn && game->grid[start_y + i + 2][start_x + i + 2] == game->turn && game->grid[start_y + i + 3][start_x + i + 3] == game->turn)
            {
                printf("Vittoria diagonale!\n");
                game->winner = game->turn;
                return;
            }
        }
    }

    printf("Controllo diagonale inverso...\n");
    for (int offset = -3; offset <= 0; offset++)
    {
        int start_y = y + offset;
        int start_x = x - offset;
        if (start_y < 0 || start_y + 3 >= 6 || start_x < 0 || start_x + 3 >= 7)
            continue;
        for (int i = 0; i < 4; i++)
        {
            if (game->grid[start_y + i][start_x - i] == game->turn && game->grid[start_y + i + 1][start_x - i - 1] == game->turn && game->grid[start_y + i + 2][start_x - i - 2] == game->turn && game->grid[start_y - i - 3][start_x + i + 3] == game->turn)
            {
                printf("Vittoria diagonale inversa!\n");
                game->winner = game->turn;
                return;
            }
        }
    }

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 7; j++)
            if (game->grid[i][j] == EMPTY)
                return;
    }

    game->winner = 2;
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[MAX_BUFFER_SIZE];
    Game game;

    init_game(&game);
    print_grid(buffer, &game);
    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <port server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Creazione socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Inizializzazione indirizzo server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                // Dominio
    server_addr.sin_port = htons(atoi(argv[1]));     // Porta
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Indirizzo IP

    // Binding
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    // Listen
    if (listen(sockfd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Accettazione connessione
        if ((game.players[i].sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Ricezione nome giocatore
        if (recv(game.players[i].sockfd, game.players[i].name, MAX_NAME_SIZE, 0) < 0)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("Player %s connected\n", game.players[i].name);
    }

    while (game.winner == -1)
    {

        // Invio griglia
        print_grid(buffer, &game);
        if (send(game.players[game.turn].sockfd, buffer, strlen(buffer) + 1, 0) < 0)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }

        printf("Sent grid to player %s\n", game.players[game.turn].name);

        // Ricezione mossa
        int column;
        if (recv(game.players[game.turn].sockfd, &column, sizeof(column), 0) < 0)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("Player %s moved in column %d\n", game.players[game.turn].name, column);

        // Controllo mossa, che sia all'interno delle colonne o che la colonna non sia piena
        if (column < 1 || column > 7 || game.grid[5][column - 1] != EMPTY)
        {
            printf("Invalid move\n");
            continue;
        }

        // Inserimento pedina
        int y;
        int x = column - 1;
        for (int i = 0; i < 6; i++)
        {
            if (game.grid[i][x] == EMPTY)
            {
                game.grid[i][x] = game.turn;
                y = i;
                break;
            }
        }

        // Controllo vittoria
        get_winner(&game, x, y);
        // Cambio turno
        game.turn = (game.turn + 1) % MAX_PLAYERS;
    }

    // Notifica il vincitore
    print_grid(buffer, &game);
    printf("%s", buffer);
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        print_grid(buffer, &game);
        if (game.winner == i)
            strcat(buffer, "Hai vinto!\n");
        else if (game.winner == 1 - i)
            strcat(buffer, "Hai perso!\n");
        else
            strcat(buffer, "Pareggio!\n");
        if (send(game.players[i].sockfd, buffer, strlen(buffer) + 1, 0) < 0)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }

    close(sockfd);
    return 0;
}