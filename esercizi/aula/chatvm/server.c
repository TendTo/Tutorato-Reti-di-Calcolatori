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

typedef enum
{
    C,
    CPP,
    JAVA,
    PYTHON,
    MATLAB,
    R
} Languages;

void init_language_matrix(int language_matrix[N_LANGUAGES][N_LANGUAGES])
{
    for (int i = 0; i < N_LANGUAGES; i++)
        for (int j = 0; j <= i; j++)
            language_matrix[i][j] = language_matrix[j][i] = i == j ? 0 : (rand() % 245) + 10;
}

void print_matrix(int language_matrix[N_LANGUAGES][N_LANGUAGES])
{
    for (int i = 0; i < N_LANGUAGES; i++)
    {
        for (int j = 0; j < N_LANGUAGES; j++)
            printf("%d ", language_matrix[j][i]);
        printf("\n");
    }
}

int language_to_int(char language[])
{
    if (strcmp(language, "c") == 0)
        return C;
    if (strcmp(language, "c++") == 0)
        return CPP;
    if (strcmp(language, "java") == 0)
        return JAVA;
    if (strcmp(language, "R") == 0)
        return R;
    if (strcmp(language, "python") == 0)
        return PYTHON;
    if (strcmp(language, "matlab") == 0)
        return MATLAB;
}

char *int_to_language(int language)
{
    switch (language)
    {
    case C:
        return "c";
    case CPP:
        return "c++";
    case JAVA:
        return "java";
    case PYTHON:
        return "python";
    case R:
        return "R";
    case MATLAB:
        return "matlab";
    default:
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Ogni volta che un nuovo client si registra, verrà aggiunta una riga al file di database
 * nel formato
 * <linguaggio> <porta>
 * @param language
 * @param port
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

void find_closest_languages(int language_matrix[N_LANGUAGES][N_LANGUAGES], int starting_language, int n, char *closest_languages[])
{
    int indexes[n];

    printf("Avvio closest languages: starting %d, n: %d\n", starting_language, n);

    for (int i = 0; i < n; i++)
    {
        int min_dist = __INT_MAX__;
        int index = -1;
        for (int j = 0; j < N_LANGUAGES; j++)
        {
            int already_visited = 0;
            if (starting_language == j)
                continue;

            for (int k = 0; k < i; k++)
                if (j == indexes[k])
                    already_visited = 1;
            if (already_visited)
                continue;

            int current_dist = language_matrix[starting_language][j];
            if (current_dist < min_dist)
            {
                min_dist = current_dist;
                index = j;
            }
        }
        indexes[i] = index;
    }

    printf("Fine closest language: %d\n", indexes[0]);
    for (int i = 0; i < n; i++)
        closest_languages[i] = int_to_language(indexes[i]);
}

void send_message_to_languages(int n, char *languages[], char message[])
{
    struct sockaddr_in client_addr;

    FILE *database = fopen(DATABASE_FILE, "r");
    char line[LINE_SIZE];

    while (fgets(line, LINE_SIZE, database))
    {
        char *language = strtok(line, " ");
        for (int i = 0; i < n; i++)
        {
            if (strcmp(language, languages[i]) != 0)
                continue;

            char *ip = strtok(NULL, " ");
            int port = atoi(strtok(NULL, " "));

            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_addr.sin_family = AF_INET;
            inet_pton(AF_INET, ip, &client_addr.sin_addr);
            client_addr.sin_port = htons(port);

            int sockfd;
            debug(sockfd = socket(AF_INET, SOCK_STREAM, 0));
            debug(connect(sockfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)));

            debug(send(sockfd, message, strlen(message) + 1, 0));
        }
    }
}

void child_process(int sockfd, struct sockaddr_in *client_addr, int languages_matrix[N_LANGUAGES][N_LANGUAGES])
{
    char *port, *language, reg_msg[MSG_SIZE], ip[INET_ADDRSTRLEN], msg[MSG_SIZE];

    memset(reg_msg, 0, MSG_SIZE);
    memset(ip, 0, INET_ADDRSTRLEN);

    debug(recv(sockfd, reg_msg, MSG_SIZE - 1, 0));
    printf("Messaggio ricevuto dal client: %s\n", reg_msg);

    language = strtok(reg_msg, " ");
    port = strtok(NULL, " ");
    inet_ntop(AF_INET, &client_addr->sin_addr, ip, sizeof(struct sockaddr_in));

    save_on_file(language, ip, port);

    while (1)
    {
        debug(recv(sockfd, msg, MSG_SIZE - 1, 0));

        int n = atoi(strtok(msg, " "));
        char *message = strtok(NULL, " ");

        printf("Messaggio ricevuto: %s\nn: %d, messaggio %s\n", msg, n, message);

        char *closest_languages[n];
        find_closest_languages(languages_matrix, language_to_int(language), n, closest_languages);

        for (int i = 0; i < n; i++)
            printf("linguaggio %i: %s\n", i, closest_languages[i]);

        send_message_to_languages(n, closest_languages, message);
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
