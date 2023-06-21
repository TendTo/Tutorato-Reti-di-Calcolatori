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
#define PORT_SIZE 10
#define LANGUAGE_SIZE 30
#define DATABASE_FILE "database.txt"
#define N_LANGUAGES 6
#define LINE_SIZE 124
#define C_STR "c"
#define CPP_STR "c++"
#define JAVA_STR "java"
#define PYTHON_STR "python"
#define R_STR "R"
#define MATLAB_STR "matlab"

typedef enum
{
    C,
    CPP,
    JAVA,
    PYTHON,
    MATLAB,
    R
} Languages;

/**
 * @brief Inizializza la matrice delle distanze di ogni linguaggio.
 * Si tratta di una matrice simmetrica, per cui la trasposta della matrice è uguale alla matrice stessa.
 * La diagonale contiene solo 0.
 * Esempio:
 * 0 1 2 3
 * 1 0 4 5
 * 2 4 0 6
 * 3 5 6 0
 * @param language_matrix matrice da inizializzare. Avrà dimensioni N_LANGUAGES x N_LANGUAGES
 */
void init_language_matrix(int language_matrix[N_LANGUAGES][N_LANGUAGES])
{
    for (int i = 0; i < N_LANGUAGES; i++)
        for (int j = 0; j < i; j++) // j < i per evitare di inizializzare due volte la stessa cella. Inoltre salta la diagonale
            language_matrix[i][j] = language_matrix[j][i] = (rand() % 245) + 10;
}

/**
 * @brief Stampa la matrice dei linguaggi a scopo di debug
 * @param language_matrix matrice N_LANGUAGES x N_LANGUAGES da stampare
 */
void print_matrix(int language_matrix[N_LANGUAGES][N_LANGUAGES])
{
    for (int i = 0; i < N_LANGUAGES; i++)
    {
        for (int j = 0; j < N_LANGUAGES; j++)
            printf("%d\t", language_matrix[j][i]);
        printf("\n");
    }
}

/**
 * @brief Converte una string di un linguaggio nel corrispondente intero, definito nell'enum @ref Languages
 * @param language stringa che contiene il nome del linguaggio
 * @return Languages corrispondente
 */
int language_to_int(const char language[])
{
    if (strcmp(language, C_STR) == 0)
        return C;
    if (strcmp(language, CPP_STR) == 0)
        return CPP;
    if (strcmp(language, JAVA_STR) == 0)
        return JAVA;
    if (strcmp(language, R_STR) == 0)
        return R;
    if (strcmp(language, PYTHON_STR) == 0)
        return PYTHON;
    if (strcmp(language, MATLAB_STR) == 0)
        return MATLAB;
    fprintf(stderr, "Linguaggio non riconosciuto: %s\n", language);
    exit(EXIT_FAILURE);
}

/**
 * @brief Converte un intero preso dall'enum @ref Languages in una stringa che contiene il nome del linguaggio
 * @param language intero che rappresenta il linguaggio
 * @return stringa che contiene il nome del linguaggio
 */
char *int_to_language(int language)
{
    switch (language)
    {
    case C:
        return C_STR;
    case CPP:
        return CPP_STR;
    case JAVA:
        return JAVA_STR;
    case PYTHON:
        return PYTHON_STR;
    case R:
        return R_STR;
    case MATLAB:
        return MATLAB_STR;
    default:
        fprintf(stderr, "Codice linguaggio non riconosciuto: %d\n", language);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Rimuovi l'utente dal database, se presente.
 * La rimozione avviene limitandosi a non copiare tutte le righe che hanno come ip quello indicato.
 * @param ip indirizzo ip dell'utente da rimuovere
 */
void remove_from_file(char client_ip[])
{
    FILE *database = fopen(DATABASE_FILE, "r");
    FILE *tmp = fopen("tmp.txt", "w");
    if (database == NULL || tmp == NULL)
    {
        perror("remove_from_file");
        exit(EXIT_FAILURE);
    }

    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, database) != NULL)
    {
        char *language = strtok(line, " ");
        char *ip = strtok(NULL, " ");
        if (strcmp(ip, client_ip) != 0)
            fputs(line, tmp);
    }
    fclose(database);
    fclose(tmp);
    remove(DATABASE_FILE);
    rename("tmp.txt", DATABASE_FILE);
}

/**
 * @brief Ogni volta che un nuovo client si registra, verrà aggiunta una riga al file di database
 * nel formato
 * <linguaggio> <ip> <porta>
 * @param language linguaggio con cui il client si è registrato
 * @param ip indirizzo ip del client
 * @param port porta da usare per contattare il client
 */
void save_on_file(char language[], char ip[], char port[])
{
    FILE *database = fopen(DATABASE_FILE, "a");
    if (database == NULL)
    {
        perror("database");
        exit(EXIT_FAILURE);
    }
    fprintf(database, "%s %s %s\n", language, ip, port);
    fclose(database);
}

/**
 * @brief Trova gli @p n linguaggi più vicini a @p starting_language seguendo la matrice delle distanze @p language_matrix
 * @param language_matrix matrice delle distanze fra i linguaggi
 * @param starting_language linguaggio di partenza
 * @param n numero di linguaggi da trovare
 * @param closest_languages array di stringhe che conterrà gli @p n linguaggi più vicini a @p starting_language
 */
void find_closest_languages(int language_matrix[N_LANGUAGES][N_LANGUAGES], const char starting_language[], int n, char *closest_languages[])
{
    printf("Ricerca degli %d linguaggi piu' vicini a partire da %s\n", n, starting_language);

    int language_row = language_to_int(starting_language);
    int indexes[n];

    for (int i = 0; i < n; i++)
    {
        int min_dist = __INT_MAX__;
        int index = -1;
        for (int j = 0; j < N_LANGUAGES; j++)
        {
            if (language_row == j) // salta il linguaggio di partenza
                continue;

            int already_visited = 0; // controlla che il linguaggio non sia già stato selezionato
            for (int k = 0; k < i; k++)
                if (j == indexes[k])
                    already_visited = 1;
            if (already_visited)
                continue;

            int current_dist = language_matrix[language_row][j];
            if (current_dist < min_dist)
            {
                // se la distanza è minore del minimo corrente, aggiorna il minimo
                // e imposta l'indice con quello del linguaggio candidato
                min_dist = current_dist;
                index = j;
            }
        }
        // salva l'indice del linguaggio più vicino
        indexes[i] = index;
    }

    // converte gli indici nelle stringhe corrispondenti e li mette nell'array closest_languages
    for (int i = 0; i < n; i++)
        closest_languages[i] = int_to_language(indexes[i]);
}

/**
 * @brief Invia un messaggio a tutti i client che si sono registrati con uno qualsiasi fra i linguaggi contenuti in @p languages
 * @param languages array di stringhe che contiene i linguaggi da contattare
 * @param n numero di linguaggi contenuti in @p languages
 * @param message messaggio da inviare
 */
void send_message_to_languages(char *languages[], int n, char message[])
{
    struct sockaddr_in client_addr;
    char line[LINE_SIZE];

    FILE *database = fopen(DATABASE_FILE, "r");

    while (fgets(line, LINE_SIZE, database)) // legge il database riga per riga
    {
        char *language = strtok(line, " "); // estrae il linguaggio
        for (int i = 0; i < n; i++)         // per ogni linguaggio da contattare
        {
            if (strcmp(language, languages[i]) != 0) // controlla che il linguaggio corrente sia quello da contattare
                continue;

            char *ip = strtok(NULL, " ");       // estrae l'ip
            int port = atoi(strtok(NULL, " ")); // estrae la porta

            // inizializza la struttura per la connessione con i dati del client
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_addr.sin_family = AF_INET;
            inet_pton(AF_INET, ip, &client_addr.sin_addr);
            client_addr.sin_port = htons(port);

            int sockfd;
            debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
            if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)) < 0)
            {
                // Non è possibile contattare il client (magari si è spento dopo essersi registrato)
                perror("connect");
                close(sockfd);
                continue;
            }

            debug(send(sockfd, message, strlen(message) + 1, 0)); // invia il messaggio
            close(sockfd);
        }
    }

    fclose(database);
}

void child_process(int sockfd, struct sockaddr_in *client_addr, int languages_matrix[N_LANGUAGES][N_LANGUAGES])
{
    char *port, *language, reg_msg[MSG_SIZE], ip[INET_ADDRSTRLEN], msg[MSG_SIZE];

    memset(reg_msg, 0, MSG_SIZE);
    memset(ip, 0, INET_ADDRSTRLEN);
    memset(msg, 0, MSG_SIZE);

    // riceve il messaggio di registrazione '<linguaggio> <porta>'
    debug(recv(sockfd, reg_msg, MSG_SIZE - 1, 0));
    printf("Messaggio di registrazione ricevuto dal client: %s\n", reg_msg);

    language = strtok(reg_msg, " ");                                            // estrae il linguaggio
    port = strtok(NULL, " ");                                                   // estrae la porta
    inet_ntop(AF_INET, &client_addr->sin_addr, ip, sizeof(struct sockaddr_in)); // estrae l'ip

    remove_from_file(ip);             // rimuove il client dal database, se presente (impedisce di fare prove in locale)
    save_on_file(language, ip, port); // salva il client nel database, con linguaggio, ip e porta

    while (1)
    {
        // attende il messaggio '<n> <messaggio>'
        debug(recv(sockfd, msg, MSG_SIZE - 1, 0));

        int n = atoi(strtok(msg, " "));
        char *message = strtok(NULL, " ");

        printf("Messaggio da inoltrare ai %d linguaggi più vicini: %s\n", n, message);

        if (n < 0 || n > N_LANGUAGES)
        {
            fprintf(stderr, "Errore: identificativo di linguaggio non valido\n");
            continue;
        }

        char *closest_languages[n]; // array che conterrà gli n linguaggi più vicini
        find_closest_languages(languages_matrix, language, n, closest_languages);

        for (int i = 0; i < n; i++)
            printf("Linguaggio vicino %i: %s\n", i, closest_languages[i]);

        send_message_to_languages(closest_languages, n, message);
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in);

    int languages_matrix[N_LANGUAGES][N_LANGUAGES] = {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}};
    init_language_matrix(languages_matrix);
    print_matrix(languages_matrix);

    if (argc < 2 || atoi(argv[1]) == 0)
    {
        fprintf(stderr, "use: %s <porta server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Mi assicuro che il file esiste
    FILE *database = fopen(DATABASE_FILE, "a");
    if (database == NULL)
    {
        perror("database");
        exit(EXIT_FAILURE);
    }
    fclose(database);

    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
    debug(bind(sockfd, (struct sockaddr *)&server_addr, len));
    debug(listen(sockfd, 10));
    int new_sockfd;

    while ((new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) > 0)
    {
        if (!fork()) // Figlio
        {
            close(sockfd);
            child_process(new_sockfd, &client_addr, languages_matrix);
            close(new_sockfd);
            exit(EXIT_SUCCESS);
        }
        close(new_sockfd);
    }

    return 0;
}
