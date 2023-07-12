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
// Se si utilizzano ip link local
server_addr.sin6_scope_id = if_nametoindex("enp0s3");
```

<!-- New subsection -->

### Client IPv6

```c
struct sockaddr_in6 = server_addr;
// ...
server_addr.sin6_family = AF_INET6;
server_addr.sin6_port = htons(atoi(argv[2]));
inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);
// Se si utilizzano ip link local
server_addr.sin6_scope_id = if_nametoindex("enp0s3");
```

<!-- New section -->

## Socket

Gestione di socket UDP o TCP.

<!-- New subsection -->

### Impostazioni della socket

```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
int optval[1] = {1};
size_t optlen = sizeof(optval);
// Abilita il riutilizzo della socket. Evita l'errore "Address already in use"
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, optval, optlen);
// Ottiene lo stato di una impostazione sulla socket.
// Se diversa da 0, l'opzione è abilitata
getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, optval, &optlen);
if (optval != 0)
    print("SO_REUSEADDR enabled on sockfd %d!\n", sockfd);
```

<!-- New subsection -->

#### Opzioni comuni di `setsockopt`

| Opzione        | Descrizione                                  | Esempio di valore            |
| -------------- | -------------------------------------------- | ---------------------------- |
| `SO_REUSEADDR` | Abilita il riutilizzo dell'indirizzo locale. | `int optval[1] = {1}`        |
| `SO_BROADCAST` | Abilita il broadcasting sulla socket.        | `int optval[1] = {1}`        |
| `SO_RCVTIMEO`  | Timeout di ricezione.                        | `struct timeval tv = {5, 0}` |
| `SO_SNDTIMEO`  | Timeout di invio.                            | `struct timeval tv = {5, 0}` |

<!-- New subsection -->

### Server UDP

```c
struct sockaddr_in server_addr, client_addr;
socklen_t addr_len = sizeof(client_addr);
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
// ...
recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, &addr_len);
sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr, addr_len);
// ...
close(sockfd);
```

<!-- New subsection -->

### Client UDP

```c
struct sockaddr_in server_addr;
socklen_t addr_len = sizeof(server_addr);
// Inizializzazione di server_addr
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// ...
sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&server_addr, addr_len);
recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&server_addr, &addr_len);
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

#### Server TCP con fork

```c
int sockfd, new_sockfd;
// ...
while ((new_sockfd = connect(sockfd, (struct sockaddr *)&client_addr, &len)) > 0)
{
    int pid = fork();
    if (pid == 0) // processo figlio
    {
        close(sockfd);      // chiusura della socket dell'accept
        // TODO: gestione della connessione
        close(new_sockfd);  // chiusura della nuova socket
        exit(EXIT_SUCCESS); // terminazione del processo figlio
    }
    else if (pid > 0) // processo padre
    {
        close(new_sockfd); // chiusura della nuova socket, torna all'accept
        continue;
    }
}
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

<!-- New subsection -->

### Broadcasting su subnet

```c
struct sockaddr_in broadcast_addr;
size_t addr_len = sizeof(broadcast_addr);
// Indirizzo di broadcast nella subnet 192.168.1.0/24
inet_pton(AF_INET, "192.168.1.255", &broadcast_addr.sin_addr);
//...
// Abilita il broadcasting sulla socket
int optval[1] = {1};
size_t optlen = sizeof(optval);
setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, optval, optlen);
//...
sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&broadcast_addr, addr_len);
```

<!-- New subsection -->

### Broadcasting su rete

```c
struct sockaddr_in broadcast_addr;
size_t addr_len = sizeof(broadcast_addr);
// Indirizzo di broadcast generico: 255.255.255.255
broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;
//...
// Abilita il broadcasting sulla socket
int optval[1] = {1};
size_t optlen = sizeof(optval);
setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, optval, optlen);
// Effettua il binding della socket su una scheda di rete
char netif[1] = "eth0";
setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &netif, sizeof(netif));
//...
sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&broadcast_addr, addr_len);
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

<!-- New subsection -->

### Lettura di un carattere

```c
char c = getchar();
getchar(); // Rimuove \n dallo standard input
```

<!-- New subsection -->

### Stampa di una stringa

```c
printf("Hello, world!\n");
// Stampa di stringhe con variabili
char name[1024] = "Mario";
printf("Hello, %s!\n", name);
// Interi, float e double
int age = 24; float height = 1.80; double weight = 70.5;
printf("age: %d, height: %f, weight: %lf\n", age, height, weight);
```

<!-- New subsection -->

### Stampa su altri stream

```c
// Stampa sullo standard error invece che sullo standard output.
// stderr è un FILE *, e potrebbe essere qualsiasi altro stream, inclusi file.
fprintf(stderr, "Errore!\n");
// Supporta le stesse funzioni di printf
fprintf(stderr, "age: %d, height: %f, weight: %lf\n", age, height, weight);
```

<!-- New section -->

## File

Gestione dei file.

<!-- New subsection -->

### Lettura riga per riga

```c
FILE *fp = fopen("file.txt", "r");
char line[1024];
while (fgets(line, sizeof(line), fp) != NULL) {
    // ...
}
fclose(fp);
```

<!-- New subsection -->

### Lettura carattere per carattere

```c
FILE *fp = fopen("file.txt", "r");
char c;
while ((c = fgetc(fp)) != EOF) {
    // ...
}
fclose(fp);
```

<!-- New subsection -->

### Lettura a blocchi

```c
FILE *fp = fopen("file.txt", "r");
char buffer[1024];
while (fread(buffer, sizeof(char), 1024 fp)) {
    // ...
}
fclose(fp);
```

<!-- New subsection -->

### Scrittura stringa

```c
FILE *fp = fopen("file.txt", "w");
char str[] = "Hello, world!\n";
fputs(str, fp);
fclose(fp);
```

<!-- New subsection -->

### Scrittura formattata

```c
// "w" sovrascrive l'intero file, usare "a" per append
FILE *fp = fopen("file.txt", "w");.
fprintf(fp, "Hello, world! I am %d years old!\n", 24);
fclose(fp);
```

<!-- New subsection -->

### Scrittura a blocchi

```c
FILE *fp = fopen("file.txt", "w");
char buffer[1024];
fwrite(buffer, sizeof(char), 1024, fp);
fclose(fp);
```

<!-- New subsection -->

### Assicurarsi che un file esista

```c
FILE *fp = fopen("file.txt", "a");
if (fp == NULL)
{
    perror("fopen");
    exit(EXIT_FAILURE);
}
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
    fputs(line, fp_tmp);
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
// La funzione strtok modifica la stringa originale
// aggiungendo \0 al posto del token!
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
// Copia solo i primi n caratteri
strncpy(str, "Hello, world!", 5); // str = "Hello"
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
// str = "Hello, world! I am 24 years old!"
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

## SSH

Gestione di SSH.

<!-- New subsection -->

### Connessione a un server SSH

```shell
ssh <user>@<ip>
# Esempio
ssh root@192.168.56.101
```

<!-- New subsection -->

### Copia di file da locale a remoto

```shell
scp <file> <user>@<ip>:<path>
# Esempio, il file server.c viene copiato nella cartella /home/user
scp server.c user@192.168.56.102:
# Esempio, il file client.c viene copiato nella cartella /home/user/compiti
scp client.c root@192.168.56.102:/home/user/compiti
```

<!-- New subsection -->

### Copia di file da remoto a locale

```shell
scp <user>@<ip>:<path> <file>
# Esempio, il file server.c viene copiato nella cartella in cui si trova il terminale
scp user@192.168.56.101:server.c .
```

<!-- New subsection -->

### Forwarding di porte

```shell
ssh -L <porta_locale>:localhost:<porta_remota> <user>@<ip>
# Esempio, la porta 8080 in locale viene mappata sulla porta 80 della macchina remota
ssh -L 8080:localhost:80 root@192.168.56.101
```

<!-- New subsection -->

### Riavvio del servizio SSH

```shell
systemctl restart ssh
```

<!-- New section -->

## Virtualbox script

Script per la creazione e gestione di macchine virtuali con Virtualbox.

<!-- New subsection -->

### Creazione di una macchina virtuale

```shell
# Creazione della macchina virtuale da 0
vboxmanage createvm --name "Debian" --ostype "Debian_64" --register
vboxmanage modifyvm "Debian" --memory 1024 --vram 128 --cpus 2
vboxmanage storagectl "Debian" --name "SATA Controller" --add sata --controller IntelAHCI
vboxmanage storageattach "Debian" --storagectl "SATA Controller" --port 0 --device 0 --type hdd --medium "Debian.vdi"
vboxmanage storagectl "Debian" --name "IDE Controller" --add ide
vboxmanage storageattach "Debian" --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium "debian-10.3.0-amd64-netinst.iso"
vboxmanage modifyvm "Debian" --boot1 dvd --boot2 disk --boot3 none --boot4 none
vboxmanage modifyvm "Debian" --nic1 bridged --bridgeadapter1 "Intel(R) Dual Band Wireless-AC 8265"
vboxmanage startvm "Debian"
```

<!-- New subsection -->

### Clone di una macchina virtuale

```shell
# Clone della macchina virtuale. Clone completo.
vboxmanage clonevm "Debian" --name "Debian2" --register
vboxmanage startvm "Debian2"
# Clone della macchina virtuale. Clone linkato.
vboxmanage snapshot "Debian" take "DebianSnapshot" --description "DebianSnapshot"
vboxmanage clonevm "Debian" --snapshot "DebianSnapshot" --name "Debian3" --options link --register
vboxmanage startvm "Debian3"
```

<!-- New subsection -->

### Gestione di una macchina virtuale

```shell
# Avvio della macchina virtuale
vboxmanage startvm "Debian"
# Arresto della macchina virtuale
vboxmanage controlvm "Debian" poweroff
# Sospensione della macchina virtuale
vboxmanage controlvm "Debian" savestate
# Riavvio della macchina virtuale
vboxmanage controlvm "Debian" reset
# Rimozione della macchina virtuale
vboxmanage unregistervm "Debian" --delete
# Rimozione dello snapshot
vboxmanage snapshot "Debian" delete "DebianSnapshot"
```

<!-- New subsection -->

### Eseguire comandi sulla macchina virtuale

```shell
# Esecuzione di un comando sulla macchina virtuale
vboxmanage guestcontrol "Debian" run --exe "/bin/ls" --username "root" --password "root" --wait-stdout --wait-stderr -- ls -l .
# Copia di un file sulla macchina virtuale (guest additions)
vboxmanage guestcontrol "Debian" copyto --username "root" --password "root" "host/file.txt" "/vm/file.txt"
# Copia di un file dalla macchina virtuale (guest additions)
vboxmanage guestcontrol "Debian" copyfrom --username "root" --password "root" "/vm/file.txt" "host/file.txt"
```

<!-- New subsection -->

### Gestione delle interfacce di rete

```shell
# Creazione di una nuova interfaccia di rete
vboxmanage modifyvm "Debian" --nic1 intnet --intnet1 lan1 --nic2 intnet --intnet2 lan2
# Mettere up una interfaccia di rete
vboxmanage guestcontrol "Debian" run --exe /sbin/ip --username "root" --password "root" --wait-stdout -- ip link set enp0s8 up
# Impostare un indirizzo ip su una interfaccia di rete
vboxmanage guestcontrol "Debian" run --exe /sbin/ip  --username root --password root --wait-stdout -- ip addr add 192.168.1.254/24 dev enp0s8
# Abilitare il forwarding su una macchina virtuale
vboxmanage guestcontrol "Debian" run --exe /usr/sbin/sysctl --username "root" --password "root" --wait-stdout -- sysctl -w net.ipv4.ip_forward=1
```

<!-- New section -->

## Varie ed eventuali

Alcuni snippet vari un po' più avanzati che potrebbero tornare utili.

<!-- New subsection -->

### Ordinare un array di interi

```c
#include <stdlib.h>

int compare(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

int main()
{
    int array[5] = {3, 1, 0, 2, 4};
    qsort(array, 5, sizeof(int), compare);
    // qsort(array, sizeof(array) / sizeof(*array), sizeof(*array), compare);
}
```

<!-- New subsection -->

### Ordinare un array di stringhe

```c
#include <stdlib.h>
#include <string.h>

int compare(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

int main()
{
    char *array[6] = {"ciao", "come", "stai", "amico" "mio" "caro"};
    qsort(array, 6, sizeof(char*), compare);
    // qsort(array, sizeof(array) / sizeof(*array), sizeof(*array), compare);
}
```

<!-- New subsection -->

### Signup con username univoco

Ogni riga contiene un username, un ip e una porta.  
L'username deve essere univoco.

```shell
# database.txt
user1 192.168.1.2 3000
```

```c
int signup(const char username[], const char ip[], int port)
{
    char line[128];
    FILE *db = fopen("database.txt", "r+"); // Controllo che db != NULL
    while (fgets(line, sizeof(line), db))
        if (strcmp(username, strtok(line, " ")) == 0) // Trovato conflitto
        {
            fclose(db);
            return 0; // Username già presente, operazione fallita
        }
    fprintf(db, "%s %s %d\n", username, ip, port); // Registrazione utente
    fclose(db);
    return 1; // Utente registrato con successo
}
```

<!-- New subsection -->

### Controllare se un ip appartiene alla subnet di una delle interfacce di rete

```c
#include <ifaddrs.h>
int is_in_same_lan(char ip[]){
    struct ifaddrs *ifaddr, *ifa;
    getifaddrs(&ifaddr); // Controllare che l'output non sia -1!
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        int mask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
        int ip_server = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
        if ((ip & mask) == (ip_server & mask)) {
            freeifaddrs(ifaddr);
            return 1;
        }
    }
    freeifaddrs(ifaddr);
    return 0;
}
```

<!-- New subsection -->

### Macro per debug

```c
#ifdef DEBUG
#include <error.h>
#include <errno.h>
#define debug(fun)                                                              \
    do                                                                          \
    {                                                                           \
        int __RES__ = fun;                                                      \
        if (__RES__ < 0)                                                        \
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
