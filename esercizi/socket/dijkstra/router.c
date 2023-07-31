/**
 * @file router.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Sia dato un grafo di router numerati.
 * Ogni router ha un suo IP nella forma 1.1.x.x/24 verso la sua rete locale.
 * Tutti i router conoscono il grafo, che è rappresentato tramite una matrice di adiacenza.
 * Tutti pesi sono pari ad 1.
 * La matrice è uguale per tutti i router.
 * Ogni router conosce l'indirizzo dell'interfaccia verso la rete locale di ogni altro router.
 * Per semplicità, si può assumere che l'interfaccia verso la rete locale del router n abbia indirizzo 1.1.n.n.
 * Scrivere un programma per riempire le tabelle di routing.
 *
 * Nota 1: la matrice di adiacenza è scritta in modo statico nel codice.
 * Nota 2: ogni nodo conosce l'indirizzo dei suoi nodi adiacenti
 * Nota 3: in ogni nodo possono esserci impostazioni statiche locali (es: indirizzi dei router adiacenti)
 * Nota 4: per eseguire un comando esterno al programma, è possibile usare la funzione system()
 * @version 0.1
 * @date 2023-07-31
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>

#define N_ROUTERS 12 // Numero di router
#define S 0          // La distanza da se stesso è 0
#define N 1000000    // La distanza da un nodo non raggiungibile è infinita
#define C 1          // La distanza da un nodo raggiungibile è 1

/**
 * @brief Matrice di adiacenza che rappresenta il grafo formato dai router
 */
int adj_matrix[N_ROUTERS][N_ROUTERS] = {
//   1  2  3  4  5  6  7  8  9  10 11 12
    {S, N, C, N, N, N, C, N, N, C, N, N},
    // 2
    {N, S, N, N, N, C, N, C, N, N, N, C},
    // 3
    {C, N, S, N, N, C, C, N, N, N, N, N},
    // 4
    {N, N, N, S, N, C, N, N, C, C, N, N},
    // 5
    {N, N, N, N, S, N, N, C, N, N, C, C},
    // 6
    {N, C, C, C, N, S, N, N, N, N, N, N},
    // 7
    {C, N, C, N, N, N, S, N, N, N, N, C},
    // 8
    {N, C, N, N, C, N, N, S, N, N, N, N},
    // 9
    {N, N, N, C, N, N, N, N, S, N, C, N},
    // 10
    {C, N, N, C, N, N, N, N, N, S, N, N},
    // 11
    {N, N, N, N, C, N, N, N, C, N, S, N},
    // 12
    {N, C, N, N, C, N, C, N, N, N, N, S},
};

/**
 * @brief Stampa la matrice di adiacenza
 */
void draw_adj_matrix()
{
    printf("Adjacency matrix:\n");
    for (int i = 0; i < N_ROUTERS; i++)
    {
        for (int j = 0; j < N_ROUTERS; j++)
        {
            printf("%d ", adj_matrix[i][j] == N ? 9 : adj_matrix[i][j]);
        }
        printf("\n");
    }
}

/**
 * @brief Verifica che la matrice di adiacenza sia simmetrica
 */
void verify_adj_matrix()
{
    for (int i = 0; i < N_ROUTERS; i++)
        for (int j = 0; j < N_ROUTERS; j++)
            assert(adj_matrix[i][j] == adj_matrix[j][i]);
}

/**
 * @brief Stampa i percorsi minimi da tutti i router verso il router con ID router_id
 * @param router_id router di destinazione
 * @param prev lista dei predecessori
 */
void print_paths(int router_id, int prev[])
{
    for (int i = 0; i < N_ROUTERS; i++)
    {
        if (i == router_id - 1)
            continue;
        printf("Router %d\t->\trouter %d: ", i + 1, router_id);
        int current = prev[i];
        if (current == router_id - 1)
        {
            printf("Adiacente\n");
            continue;
        }
        int next = prev[current];
        while (next != router_id - 1)
        {
            printf("%d ", current + 1);
            current = next;
            next = prev[current];
        }
        printf("%d\n", current + 1);
    }
}

/**
 * @brief Stampa il percorso minimo che distanzia il router con ID router_id da tutti gli altri
 * @param router_id router di destinazione
 * @param dist lista delle distanze
 */
void print_min_paths(int router_id, int dist[])
{
    for (int i = 0; i < N_ROUTERS; i++)
    {
        if (i == router_id - 1)
            continue;
        printf("Router %d\t->\trouter %d: ", i + 1, router_id);
        if (dist[i] == N)
        {
            printf("Non raggiungibile\n");
            continue;
        }
        printf("%d\n", dist[i]);
    }
}

/**
 * @brief Aggiunge una voce alla tabella di routing del router
 * @param dest_id id del router di destinazione
 * @param next_hop id del router adiacente
 */
void add_routing_table_entry(int dest_id, int next_hop)
{
    char command[100];
    sprintf(command, "ip route add 10.0.%d.0/24 via 10.0.%d.%d", dest_id, next_hop, next_hop);
    printf("%s\n", command);
    // system(command); // Per effettuare delle modifiche reali, rimuovere il commento
}

/**
 * @brief Popola la tabella di routing del router
 * @param node_id node_id del router da cui partire
 * @param prev lista dei predecessori
 */
void popolate_routing_table(int node_id, int prev[])
{
    for (int i = 0; i < N_ROUTERS; i++)
    {
        if (i == node_id - 1)
            continue;
        int current = prev[i];
        if (current == node_id - 1)
        {
            add_routing_table_entry(i + 1, i + 1);
            continue;
        }
        int next = prev[current];
        while (next != node_id - 1)
        {
            current = next;
            next = prev[current];
        }
        add_routing_table_entry(i + 1, current + 1);
    }
}

/**
 * @brief Implementa l'algoritmo di Dijkstra per trovare il percorso minimo da un router a tutti gli altri.
 * Dopo aver eseguito l'algoritmo, bisogna aggiornare la tabella di routing del router.
 * Per ogni router, basta risalire il percorso minimo trovato per raggiungere il router, ed aggiungere alla tabella di routing l'indirizzo dell'interfaccia verso il router adiacente.
 * @param router_id L'ID del router da cui partire
 */
void dijkstra(int router_id)
{
    int dist[N_ROUTERS];
    int prev[N_ROUTERS];
    int visited[N_ROUTERS];

    // Inizializzazione delle strutture dati
    for (int i = 0; i < N_ROUTERS; i++)
    {
        dist[i] = N;
        prev[i] = -1;
        visited[i] = 0;
    }

    // La distanza dal router a se stesso è 0
    dist[router_id - 1] = 0;

    for (int i = 0; i < N_ROUTERS; i++)
    {
        int min = N;        // N è un valore molto grande che rappresenta l'infinito
        int min_index = -1; // -1 rappresenta l'assenza di un nodo

        for (int j = 0; j < N_ROUTERS; j++)
        {
            if (visited[j] == 0 && dist[j] <= min) // Se il nodo non è stato visitato e la sua distanza è minima
            {
                min = dist[j];
                min_index = j;
            }
        }

        visited[min_index] = 1; // Il nodo con distanza minima viene selezionato

        for (int j = 0; j < N_ROUTERS; j++)
        {
            if (visited[j] == 0 && dist[min_index] + adj_matrix[min_index][j] < dist[j]) // Ogni altro nodo non visitato viene aggiornato
            {
                dist[j] = dist[min_index] + adj_matrix[min_index][j]; // Si aggiorna la distanza
                prev[j] = min_index;                                  // Si assegna il predecessore
            }
        }
    }

    // print_min_paths(router_id, dist);
    // print_paths(router_id, prev);
    popolate_routing_table(router_id, prev);
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <router_id>\n", argv[0]);
        exit(1);
    }

    int router_id = atoi(argv[1]);
    if (router_id < 1 || router_id > N_ROUTERS)
    {
        fprintf(stderr, "error: router_id deve essere fra 1 e %d\n", N_ROUTERS);
        exit(1);
    }

    verify_adj_matrix();
    dijkstra(router_id);

    return 0;
}
