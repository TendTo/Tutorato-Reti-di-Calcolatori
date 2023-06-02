# Snippets

Collezione di snippet di codice comunemente utilizzati per reti.

<!-- New section -->

## ATTENZIONE

**ASSICURATEVI DI COMPRENDERE IL CODICE PRIMA DI UTILIZZARLO**

<!-- New section -->

## Sockaddr

Gestione di `sockaddr_in` e `sockaddr_in6` per IPv4 e IPv6.

<!-- New subsection -->

### Server IPv4

```c
struct sockaddr_in = server_addr;
// ...
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(atoi(argv[1]));
server_addr.sin_addr.s_addr = INADDR_ANY;
```

<!-- New subsection -->

### Client IPv4

```c
struct sockaddr_in = server_addr;
// ...
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(atoi(argv[2]));
inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
```

<!-- New subsection -->

### Server IPv6

```c
struct sockaddr_in6 = server_addr;
// ...
server_addr.sin6_family = AF_INET6;
server_addr.sin6_port = htons(atoi(argv[1]));
server_addr.sin6_addr = in6addr_any;
```

<!-- New subsection -->

### Client IPv6

```c
struct sockaddr_in6 = server_addr;
// ...
server_addr.sin6_family = AF_INET6;
server_addr.sin6_port = htons(atoi(argv[2]));
inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);
```

<!-- New section -->

## Socket

Gestione di socket UDP o TCP.

<!-- New subsection -->

### Server UDP

```c
struct sockaddr_in server_addr, client_addr;
socklen_t client_len = sizeof(client_addr);
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
// ...
recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, client_len);
// ...
close(sockfd);
```

<!-- New subsection -->

### Client UDP

```c
struct sockaddr_in server_addr;
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// ...
sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
// ...
close(sockfd);
```

<!-- New subsection -->

### Server TCP

```c
struct sockaddr_in server_addr, client_addr;
socklen_t client_len = sizeof(client_addr);
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
listen(sockfd, 5);
// ...
int new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
recv(new_sockfd, buffer, sizeof(buffer), 0);
send(new_sockfd, buffer, sizeof(buffer), 0);
// ...
close(new_sockfd);
close(sockfd);
```

<!-- New subsection -->

### Client TCP

```c
struct sockaddr_in server_addr;
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
// ...
send(sockfd, buffer, sizeof(buffer), 0);
recv(sockfd, buffer, sizeof(buffer), 0);
// ...
close(sockfd);
```

<!-- New section -->

## Strutture dati ed enumerazioni

Gestione di strutture dati più complesse ed enumerazioni.

<!-- New subsection -->

### Struttura dati

```c
struct Struttura {
    char name[1024];
    int age;
};
// È possibile creare un alias per la struttura
typedef struct Struttura Struttura;
// O direttamente
typedef struct {
    char name[1024];
    int age;
} Struttura;
```

<!-- New subsection -->

### Enum

```c
enum Elenco {
    CASE_1,
    CASE_2 = 'b',
    CASE_3 = 123123
};
// È possibile creare un alias per l'enum
typedef enum Elenco Elenco;
// O direttamente
typedef enum {
    CASE_1,
    CASE_2,
    CASE_3
} Elenco;
```

<!-- New section -->

## Invio di messaggi tramite struct

Una semplice stringa potrebbe non essere sufficiente per la comunicazione, o necessitare ulteriore parsing.

Per ovviare al problema, si può valutare il vantaggio di inviare un messaggio tramite una struct, che può contenere più informazioni in maniera ordinata.

<!-- .element: class="fragment" -->

<!-- New subsection -->

### Strutture dati

```c
typedef enum {
    REGISTER = 'r',
    LOGIN = 'l',
    DELETE = 'd',
    EXIT = 'e',
} Operation;

typedef struct {
    Operation operation; // L'operazione da eseguire è un char nascosto dall'enum
    char username[1024]; // Username
    char password[1024]; // Password
} Message;
```

<!-- New subsection -->

### Invio del messaggio

```c
Message message;
// Potrei anche leggere un char da stdin con getchar()
message.operation = REGISTER;
strcpy(message.username, "username");
strcpy(message.password, "password");
send(sockfd, &message, sizeof(message), 0);
```

<!-- New subsection -->

### Ricezione del messaggio

```c
Message message;
recv(sockfd, &message, sizeof(message), 0);
switch (message.operation) { // Si decide cosa fare in base all'operazione
    case REGISTER:
        break;
    case LOGIN:
        break;
    case DELETE:
        break;
    case EXIT:
        break;
}
```

<!-- New section -->

## I/O

Gestione di standard output e input.

<!-- New subsection -->

### Lettura stringa con prompt

```c
char buffer[1024];
printf("Inserisci del testo: "); // Mostra il prompt all'utente
fgets(buffer, sizeof(buffer), stdin);
buffer[strcspn(buffer, "\n")] = '\0'; // Rimuove \n
```

<!-- New section -->

### Lettura di un carattere

```c
char c = getchar();
getchar(); // Rimuove \n dallo standard input
```

<!-- New section -->

## File

Gestione dei file.

<!-- New subsection -->

### Lettura

```c
FILE *fp = fopen("file.txt", "r");
char line[1024];
while (fgets(line, sizeof(line), fp) != NULL) {
    // ...
}
fclose(fp);
```

<!-- New subsection -->

### Scrittura

```c
// "w" sovrascrive l'intero file, usare "a" per append
FILE *fp = fopen("file.txt", "w");.
fprintf(fp, "Hello, world! I am %d years old!\n", 24);
fclose(fp);
```

<!-- New subsection -->

### Rimuovere una riga

```c
FILE *fp = fopen("file.txt", "r"); // file da cui rimuovere la riga
FILE *fp_tmp = fopen("file.tmp", "w"); // file temporaneo
char line[1024];
while (fgets(line, sizeof(line), fp) != NULL) {
    if (/* condizione per rimuovere la riga */) {
        continue;
    }
    fprintf(fp_tmp, "%s", line);
}
fclose(fp);
fclose(fp_tmp);
remove("file.txt");
rename("file.tmp", "file.txt");
```

<!-- New section -->

## Stringhe

Gestione delle stringhe.

<!-- New subsection -->

### Lunghezza di una stringa

```c
int len = strlen(str);
```

<!-- New subsection -->

### Tokenizzazione

```c
char *token = strtok(str, " ");
while (token != NULL) {
    // Token assumerà il valore di ogni parola della stringa fino a NULL
    token = strtok(NULL, " "); // NULL per continuare sulla stessa stringa
}
```

<!-- New subsection -->

### Copia di stringhe

```c
strcpy(str, "Hello, world!"); // str = "Hello, world!"
```

<!-- New subsection -->

### Concatenazione

```c
strcpy(str, "Hello, "); // str = "Hello, "
strcat(str, "world!");  // str = "Hello, world!"
```

<!-- New subsection -->

### Formattazione di stringhe

```c
sprintf(str, "Hello, %s! I am %d years old!", "world", 24);
```

<!-- New subsection -->

### Terminare la stringa prima di un carattere

```c
char str = "Hello, world!\n";
strchr(str, '\n')[0] = '\0'; // str = "Hello, world!"
// oppure
string[strcspn(str, "\n")] = '\0'; // str = "Hello, world!"
// oppure, per rimuovere l'ultimo carattere
string[strlen(str) - 1] = '\0'; // str = "Hello, world!"
```

<!-- New subsection -->

### Confronto di stringhe

```c
// Restituisce 0 se le stringhe sono uguali
int result = strcmp(str1, str2);
int result = strncmp(str1, str2, 5); // Confronta i primi 5 caratteri
int result = strcasecmp(str1, str2); // Case insensitive
// Case insensitive e confronta i primi 5 caratteri
int result = strncasecmp(str1, str2, 5);
```

<!-- New subsection -->

### Conversione da stringa a numero

```c
// Se la conversione fallisce, il valore di n sarà 0
int n = atoi(str);
long n = atol(str);
long long n = atoll(str);
```

<!-- New subsection -->

### Conversione da numero a stringa

```c
sprintf(str, "%d", n);
```

<!-- New section -->

## Processi

Gestione di processi.

<!-- New subsection -->

### Creazione di un processo

```c
pid_t pid = fork();
if (pid == 0) {
    // Processo figlio
} else if (pid > 0) {
    // Processo padre
} else {
    // Errore
}
```

<!-- New subsection -->

### Terminazione di un processo

```c
exit(0); // Termina il processo corrente
exit(1); // Termina il processo corrente con errore
```

<!-- New subsection -->

### Ottenere il proprio PID

```c
pid_t pid = getpid();
```

<!-- New subsection -->

### Attesa di terminazione di un processo figlio

```c
// Processo padre
wait(NULL); // Aspetta la terminazione di un processo figlio qualsiasi
while (wait(NULL) > 0); // Aspetta la terminazione di tutti i processi figli
waitpid(pid, NULL, 0); // Aspetta la terminazione di un processo figlio con pid
```

<!-- New section -->

## Thread

Gestione di thread.

<!-- New subsection -->

### Compilazione di un programma che utilizza i thread

Per compilare un programma che utilizza i thread, aggiungere l'opzione `-pthread` al comando `gcc`.

```bash
gcc -pthread -o main main.c
```

<!-- New subsection -->

### Creazione di un thread

```c
#include <pthread.h>
void* fun(void* arg) {
    // Codice del thread
    printf("%s\n", (char*) arg);
    return NULL;
}
int main() {
    char* arg = "Hello, world!";
    pthread_t thread;
    pthread_create(&thread, NULL, fun, arg);
    pthread_join(thread, NULL);
    return 0;
}
```

<!-- New subsection -->

### Terminazione di un thread

```c
#include <pthread.h>
void* fun(void* arg) {
    // Codice del thread
    pthread_exit(NULL);
}
```

<!-- New subsection -->

### Mutua esclusione

```c
#include <pthread.h>
// Variabile globale
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// All'interno del thread
pthread_mutex_lock(&mutex);
// Sezione critica
pthread_mutex_unlock(&mutex);

// Alla fine del programma
pthread_mutex_destroy(&mutex);
return 0;
```

<!-- New section -->

## Semafori

Gestione di semafori.

<!-- New subsection -->

### Creazione di un semaforo

```c
#include <sys/sem.h>
int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
```

<!-- New subsection -->

### Inizializzazione di un semaforo

```c
#include <sys/sem.h>
int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
int semval = 1; // Valore iniziale del semaforo
semctl(semid, 0, SETVAL, semval);
```

<!-- New subsection -->

### Operazioni su un semaforo

```c
#include <sys/sem.h>
#define semwait(semid) semop(semid, &(struct sembuf){0, -1, 0}, 1)
#define semsignal(semid) semop(semid, &(struct sembuf){0, 1, 0}, 1)

semwait(semid);
// Sezione critica
semsignal(semid);
```

<!-- New subsection -->

### Rimozione di un semaforo

```c
#include <sys/sem.h>
semctl(semid, 0, IPC_RMID);
```

<!-- New section -->

## Varie ed eventuali

Alcuni snippet vari un po' più avanzati che potrebbero tornare utili.

<!-- New subsection -->

### Macro per debug

```c
#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                                              \
    do                                                                          \
    {                                                                           \
        int res = fun;                                                          \
        if (res < 0)                                                            \
            error_at_line(EXIT_FAILURE, errno, __FILE__, __LINE__, "%s", #fun); \
    } while (0)
#else
#define debug(fun) fun
#endif
```

<!-- New subsection -->

#### Utilizzo

Permette di wrappare qualsiasi espressione one-liner o funzioni.
Se il valore di ritorno dell'espressione è minore di zero, viene interpretato come un errore e fa terminare il programma.  
In questo caso saranno fornite informazioni aggiuntive su cosa è andato storto.

```c
// Si può provare il suo funzionamento con una chiamata a socket errata
debug(socket(AF_INET, SOCK_STREAM, IPPROTO_UDP));
```

Per abilitare questa funzionalità, compilare con `-DDEBUG`.

```shell
gcc -DDEBUG -o main main.c
```

<!-- New subsection -->

### Gestione di segnali (ctrl+c)

```c
#include <signal.h>
volatile sig_atomic_t is_running = 1;

void signal_handler(int) {
    is_running = 0;
}
int main() {
    signal(SIGINT, signal_handler);
    while (is_running) {
        // ...
    }
    // Cleanup
    return 0;
}
```

## Da aggiungere

- [ ] VirtualBox scripting
- [ ] Uno snippet standard per gestire l'invio in broadcast
