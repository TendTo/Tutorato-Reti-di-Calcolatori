/**
 * @author MoonLightLucrix
 * @link https://github.com/MoonLightLucrix
*/
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<sys/file.h>
#include<libgen.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define N 4096
#define database "Utenti.txt"
#define MAX_FORM_LENGTH 32
#define S_MUTEX 0
#define S_COND_1 1
#define backlog 5
#define online 'x'
#define offline 'o'
#define sended 's'
#define unsended 'u'
#define seleziona "\x1b[35m%s@%s$ Inserisci comando: \x1b[0m"
typedef struct
{
    char username[MAX_FORM_LENGTH];
    int sockfd;
    char address[INET_ADDRSTRLEN];
    unsigned short int porta;
    bool isOnline;
    bool isSended;
} Utente;
typedef enum{OK, QUIT, ERROR=-1} Status;
typedef struct
{
    char username[MAX_FORM_LENGTH];
    char msg[N];
    unsigned long int numeroUtenti;
} Messaggio;
typedef struct
{
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
} SocketInfo;
char program[FILENAME_MAX];
int shmid;
int semid;

/**
 * @brief Funzione che viene evocata quando viene registrato un signal di tipo interrupt, elimina la memoria condivisa ed i semafori messi in condivizione tra i vari processi ed uccide tutta la famiglia dei processi.
 * @param signal Tipo di signal registrato sulla quale la funzione signal viene impostata sulla cattura, in questo caso il signal di tipo SIGINT. 
*/
void signalHandler(int signal)
{
    shmctl(shmid,IPC_RMID,NULL);
    semctl(semid,0,IPC_RMID,0);
    kill(0,SIGTERM);
}

int WAIT(int semid, int cond)
{
    struct sembuf operazioni[1]={{cond,-1,0}};
    return semop(semid,operazioni,1);
}

int SIGNAL(int semid, int cond)
{
    struct sembuf operazioni[1]={{cond,+1,0}};
    return semop(semid,operazioni,1);
}

/**
 * @brief Aggiunge un nuovo utente nel database.
 * @param utente Tutte le informazioni dell'utente.
 * @param fsd File descriptor stream del file dove vengono inserite le new entry.
*/
void fadd(Utente utente, FILE*fsd)
{
    fprintf(fsd,"%s\t%d\t%s\t%d\t%c\t%c\n",utente.username,utente.sockfd,utente.address,utente.porta,(utente.isOnline)?online:offline,(utente.isSended)?sended:unsended);
}

/**
 * @brief Trova la riga nel database in base alla chiave primaria.
 * @param key Chiave primaria da cercare, questa è univoca per ogni riga.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Viene restituito un puntatore al primo char della riga trovata, altrimenti restituisce NULL se la riga non esiste.
*/
char*ffind(char*key, FILE*fsd)
{
    rewind(fsd);
    char buffer[N];
    char*line=NULL;
    char akey[strlen(key)];
    while((line=fgets(buffer,N,fsd)))
    {
        sscanf(buffer,"%[^\t]",akey);
        if(!strcmp(key,akey))
        {
            break;
        }
    }
    return line;
}

/**
 * @brief Conta tutti gli utenti online del database.
 * @param fsd File descriptor stream del file dove vengono contate le entry.
 * @return Numero complessivo di utenti online nel database come intero senza segno a 32bit.
*/
unsigned long int fcount(FILE*fsd)
{
    rewind(fsd);
    char line[N];
    unsigned long int count=0;
    char isOnline;
    while(fgets(line,N,fsd))
    {
        sscanf(line,"%*[^\t]\t%*[^\t]\t%*[^\t]\t%*[^\t]\t%c",&isOnline);
        if(isOnline==online)
        {
            count-=-1;
        }
    }
    return count;
}

/**
 * @brief Elimina la linea richiesta come parametro se questa viene trovata.
 * @param line Linea da eliminare
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
*/
void fdelete(char*line, FILE*fsd)
{
    rewind(fsd);
    char buffer[N];
    size_t size;
    FILE*ftmp=tmpfile();
    while(fgets(buffer,N,fsd))
    {
        if(strcmp(line,buffer))
        {
            fputs(buffer,ftmp);
        }
    }
    rewind(fsd);
    rewind(ftmp);
    do{
        memset(buffer,'\0',strlen(buffer));
        size=fread(buffer,1,N,ftmp);
        fwrite(buffer,1,size,fsd);
    }while(N==size);
    ftruncate(fileno(fsd),ftell(fsd));
    fclose(ftmp);
}

/**
 * @brief Setta tutti i campi "isSended" all'interno del database a false.
 * @param fsd File descriptor stream del file dove vengono settati i campi.
*/
void flisten(FILE*fsd)
{
    rewind(fsd);
    Utente utente;
    char line[N];
    long int pos;
    size_t size;
    FILE*ftmp=tmpfile();
    char isOnline, isSended;
    printf("flisten:\n");
    while(fgets(line,N,fsd))
    {
        sscanf(line,"%[^\t]\t%d\t%[^\t]\t%hd\t%c\t%c",utente.username,&utente.sockfd,utente.address,&utente.porta,&isOnline,&isSended);
        utente.isOnline=(isOnline==online);
        utente.isSended=false;
        fadd(utente,ftmp);
    }
    rewind(ftmp);
    rewind(fsd);
    do{
        memset(line,'\0',strlen(line));
        size=fread(line,1,N,ftmp);
        fwrite(line,1,size,fsd);
    }while(size==N);
    rewind(fsd);
    while(fgets(line,N,fsd))
    {
        printf("%s",line);
    }
    fclose(ftmp);
}

/**
 * @brief Setta tutti le varie impostazioni per la socket e la struct sockaddr_in.
 * @param sockaddr SocketInfo da settare.
 * @param address Indirizzo IPv4 a cui mandare pacchetti, se viene passata come NULL verrà impostata a INADDR_ANY.
 * @param porta Porta sulla quale indirizzare i pacchetti entranti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int sockaddrSettings(SocketInfo*sockaddr, char*address, unsigned short int porta)
{
    sockaddr->addrlen=sizeof(struct sockaddr);
    if((sockaddr->sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
    {
        fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
        return -1;
    }
    memset((char*)&sockaddr->addr,0,sockaddr->addrlen);
    sockaddr->addr.sin_family=AF_INET;
    sockaddr->addr.sin_addr.s_addr=(!address)?INADDR_ANY:inet_addr(address);
    sockaddr->addr.sin_port=htons(porta);
    return 0;
}

//ReceiverFOS process -> session process

/**
 * @brief Questa funzione viene evocata dalla sessione dedicata col client prima che si metta in ascolto dei comandi mandati dal client, si mette in attesa di un messaggio inviato da un altra sessione per poterlo inoltrare al client in collegamento sulla sessione corrente a patto che il client non sia lo stesso che ha generato il messaggio.
 * @param remote Struct contenente tutte le informazioni legate alla socket.
 * @param utente Struct contenente tutte le informazioni dell'utente, che durante la funzione vengono modificate e registrate sul database.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int receiverFromOtherSessions(SocketInfo remote, Utente*utente)
{
    Messaggio*messaggio;
    FILE*fsd;
    if((messaggio=(Messaggio*)shmat(shmid,NULL,0))==(Messaggio*)-1)
    {
        fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
        return -1;
    }
    while(true)
    {
        WAIT(semid,S_COND_1);
        printf("Utente: %s\n",messaggio->username);
        if(strcmp(utente->username,messaggio->username))
        {
            if(!(fsd=fopen(database,"r+")))
            {
                SIGNAL(semid,S_COND_1);
                fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
                return -1;
            }
            flock(fileno(fsd),LOCK_EX);
            char*lineExists=ffind(utente->username,fsd);
            if(lineExists)
            {
                char line[strlen(lineExists)];
                strcpy(line,lineExists);
                char isOnline, isSended;
                sscanf(line,"%*[^\t]\t%*[^\t]\t%*[^\t]\t%*[^\t]\t%c\t%c",&isOnline,&isSended);
                if((isOnline==online) && (isSended==unsended))
                {
                    printf("Invio il messaggio a %s\n",utente->username);
                    char buffer[N];
                    memset(buffer,'\0',strlen(buffer));
                    sprintf(buffer,"\n%s: %s\n\x1b[35m%s@%s$ Inserisci comando: \x1b[0m",messaggio->username,messaggio->msg,program,utente->username);
                    if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
                    {
                        fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
                        messaggio->numeroUtenti=0;
                        SIGNAL(semid,S_MUTEX);
                        continue;
                    }
                    fdelete(line,fsd);
                    utente->isSended=true;
                    fadd(*utente,fsd);
                    messaggio->numeroUtenti--;
                }
            }
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
        }
        else
        {
            printf("L'utente %s e %s sono uguali: non faccio nulla\n",messaggio->username,utente->username);
        }
        if(messaggio->numeroUtenti>=1)
        {
            SIGNAL(semid,S_COND_1);
        }
        else
        {
            printf("numeroUtenti: %lu\n",messaggio->numeroUtenti);
            SIGNAL(semid,S_MUTEX);
        }
    }
    return 0;
}

//Session process

/**
 * @brief Funzione che manda al client l'elenco completo di tutti gli utenti connessi nell'instante in cui il client manda il messaggio "who" come comando.
 * @param remote Struct contenente tutte le informazioni legate alla socket.
 * @param fsd File descriptor stream del file usato come database.
 * @return -1 in caso di errore, 0 altrimenti.
*/
int sendUsernamesList(SocketInfo remote, FILE*fsd)
{
    char line[N];
    char username[MAX_FORM_LENGTH];
    char isOnline;
    char*token;
    dprintf(remote.sockfd,"%s: Lista utenti connessi:\n",program);
    while(fgets(line,N,fsd))
    {
        memset(username,'\0',strlen(username));
        token=strtok(line,"\t");
        strcpy(username,token);
        for(int i=1; i<=4; token=strtok(NULL,"\t"), i-=-1);
        isOnline=token[0];
        if(isOnline==offline)
        {
            continue;
        }
        sprintf(username,"%s\n",username);
        if((write(remote.sockfd,username,strlen(username)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Si occupa di inizializzare la struct condivida con i campi username del client che ha generato il messaggio, messaggio ed il numero di utenti connessi a cui mandare il messaggio ed infine risveglia il processo ReceiverFOS.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param comando Messaggio mandato dal client da inoltrare alle altre sessioni in modo tale che a loro volta lo inoltrino agli altri client connessi.
 * @param utente Struct che viene utilizzata per inizializzare la struct condivisa tra le sessioni con le varie informazioni, ad esempio, l'username che ha generato il messaggio.
 * @param fsd File descriptor stream che viene utilizzato come database.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int fowardMessagge(SocketInfo remote, char*comando, Utente utente, FILE*fsd)
{
    Messaggio*messaggio;
    if((messaggio=(Messaggio*)shmat(shmid,NULL,0))==(Messaggio*)-1)
    {
        fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
        return -1;
    }
    WAIT(semid,S_MUTEX);
    strcpy(messaggio->msg,comando);
    strcpy(messaggio->username,utente.username);
    messaggio->numeroUtenti=fcount(fsd)-1;
    printf("Numero utenti: %lu\n",messaggio->numeroUtenti);
    flock(fileno(fsd),LOCK_EX);
    flisten(fsd);
    flock(fileno(fsd),LOCK_UN);
    SIGNAL(semid,S_COND_1);
    return 0;
}

/**
 * @brief Funzione che si occupa della registrazione se l'utente è nuovo o accesso se l'utente era già registrato, non permette che due utenti siano connessi nello stesso momento.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param username Username da verificare nel database.
 * @param fsd File descriptor stream del file usato come database.
 * @return Restituisce -1 se un altro utente con lo stesso username è già connesso, 0 se la registrazione o accesso ha avuto successo.
*/
int signUp(SocketInfo remote, char*username, Utente*utente, FILE*fsd)
{
    char*lineExists=ffind(username,fsd);
    if(lineExists)
    {
        char line[strlen(lineExists)];
        strcpy(line,lineExists);
        char isOnline;
        /*char*token=strtok(line,"\t");
        for(int i=1; i<=4; i-=-1)
        {
            token=strtok(NULL,"\t");
        }
        isOnline=token[0];
        printf("token: %s\n",token);*/
        sscanf(line,"%*[^\t]\t%*[^\t]\t%*[^\t]\t%*[^\t]\t%c",&isOnline);
        if(isOnline==online)
        {
            dprintf(remote.sockfd,"%s: L'utente è già connesso\n",program);
            return -1;
        }
        else
        {
            fdelete(line,fsd);
            dprintf(remote.sockfd,"%s: Accesso eseguito\n",program);
        }
    }
    else
    {
        dprintf(remote.sockfd,"%s: Registrazione eseguita\n",program);
    }
    strcpy(utente->username,username);
    utente->sockfd=remote.sockfd;
    strcpy(utente->address,inet_ntoa(remote.addr.sin_addr));
    utente->porta=ntohs(remote.addr.sin_port);
    utente->isOnline=true;
    utente->isSended=false;
    fadd(*utente,fsd);
    return 0;
}

/**
 * @brief Effettua il log-out dell'utente dalla sessione corrente aggiornando il database che provederà a settare a offline il record.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param utente Struct contenente tutte le informazioni dell'utente con la quale aggiornare il database.
 * @param fsd File descriptor stream del file usato come database.
*/
void signOut(SocketInfo remote, Utente*utente, FILE*fsd)
{
    char*lineExists=ffind(utente->username,fsd);
    if(lineExists)
    {
        char line[strlen(lineExists)];
        strcpy(line,lineExists);
        fdelete(line,fsd);
        utente->isOnline=false;
        fadd(*utente,fsd);
        dprintf(remote.sockfd,"%s: Utente disconnesso\n",program);
    }
}

/**
 * @brief Gestisce i vari comandi mandati dal client.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param utente Struct contenente tutte le informazioni dell'utente che digita il comando.
 * @param comando Comando richiesto dall'utente.
 * @return Restituisce un tipo Status che può essere ERROR in caso di errore, OK se il comando è stato eseguito con successo o QUIT se il comando digitato dall'utente serve per uscire dalla sessione.
*/
Status commandHandler(SocketInfo remote, Utente*utente, char*comando)
{
    FILE*fsd;
    printf("comando: %s\nlength: %lu\n",comando, strlen(comando));
    if(!strcmp(comando,"who"))
    {
        if(!(fsd=fopen(database,"r")))
        {
            fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
            return ERROR;
        }
        flock(fileno(fsd),LOCK_EX);
        if((sendUsernamesList(remote,fsd))==-1)
        {
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            return ERROR;
        }
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
    }
    else if(!strcmp(comando,"quit"))
    {
        if(!(fsd=fopen(database,"r+")))
        {
            fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
            return ERROR;
        }
        flock(fileno(fsd),LOCK_EX);
        signOut(remote,utente,fsd);
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
        return QUIT;
    }
    else
    {
        if(!(fsd=fopen(database,"r+")))
        {
            fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
            return ERROR;
        }
        flock(fileno(fsd),LOCK_EX);
        if((fowardMessagge(remote,comando,*utente,fsd))==-1)
        {
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            return ERROR;
        }
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
    }
    return OK;
}

/**
 * @brief Gestisce la sessione con l'utente e ascolta tutti i comandi digitati da quest'ultimo, se un comando torna status ERROR il server slogga l'utente e termina il processo sessione, prima di mettersi in ascolto dei comandi crea un processo che si mette in ascolto di messaggi provenienti da altre sessioni del server con altri client per poter inoltrare messaggi provenienti da altri utenti.
 * @param remote Struct contenete tutte le informazioni della nuova socket di sessione con il client.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int session(SocketInfo remote)
{
    Utente utente;
    char username[MAX_FORM_LENGTH];
    char buffer[N];
    ssize_t size;
    FILE*fsd;
    memset(username,'\0',strlen(username));
    struct timeval timeout={10,0};
    if((setsockopt(remote.sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(struct timeval)))==-1)
    {
        fprintf(stderr,"%s: setsockopt: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if((size=read(remote.sockfd,username,MAX_FORM_LENGTH))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    timeout.tv_sec=0;
    if((setsockopt(remote.sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(struct timeval)))==-1)
    {
        fprintf(stderr,"%s: setsockopt: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if(!(fsd=fopen(database,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    if((signUp(remote,username,&utente,fsd))==-1)
    {
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
        close(remote.sockfd);
        return 0;
    }
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    int e=0;
    pid_t pid;
    if((pid=fork())==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if(!pid)
    {
        if((receiverFromOtherSessions(remote,&utente))==-1)
        {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    while(true)
    {
        memset(buffer,'\0',strlen(buffer));
        dprintf(remote.sockfd,seleziona,program,utente.username);
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if(!size)
        {
            continue;
        }
        Status status=commandHandler(remote,&utente,buffer);
        if(status==OK)
        {
            continue;
        }
        else
        {
            e=(status==ERROR)?1:0;
            break;
        }
    }
    if(e==-1)
    {
        if(!(fsd=fopen(database,"r+")))
        {
            fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
            close(remote.sockfd);
            return -1;
        }
        flock(fileno(fsd),LOCK_EX);
        signOut(remote,&utente,fsd);
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
    }
    //shutdown(remote.sockfd,SHUT_RDWR);
    kill(pid,SIGTERM);
    close(remote.sockfd);
    return e;
}

//Parent process

/**
 * @brief Setta tutti i vari parametri per la condivisione di memoria e per la condivisione dei semafori.
 * @param shmid ID che identifica il segmento di memoria condivisa.
 * @param semid ID che identifica i semafori condivisi.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int shmSettings(int*shmid, int*semid)
{
    key_t key=IPC_PRIVATE;
    mode_t mode=S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if((*shmid=shmget(key,sizeof(Messaggio),IPC_EXCL | IPC_CREAT | mode))==-1)
    {
        fprintf(stderr,"%s: shmget: %s\n",program,strerror(errno));
        return -1;
    }
    if((*semid=semget(key,2,IPC_EXCL | IPC_CREAT | mode))==-1)
    {
        fprintf(stderr,"%s: shmget: %s\n",program,strerror(errno));
        return -1;
    }
    semctl(*semid,S_MUTEX,SETVAL,1);
    semctl(*semid,S_COND_1,SETVAL,0);
    return 0;
}

/**
 * @brief Funzione che viene utilizzata come server nella comunicazione attende che sia effettuata l'accept per creare una sessione che continuerà la comunicazione col client, mentre il processo padre si rimette in ascolto, il protocollo utilizzato è Trasmission Control Protocol.
 * @param porta Porta sulla quale vengono reindirizzati i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int server(unsigned short int porta)
{
    SocketInfo locale, remote;
    if((sockaddrSettings(&locale,NULL,porta))==-1)
    {
        return -1;
    }
    if((bind(locale.sockfd,(struct sockaddr*)&locale.addr,locale.addrlen))==-1)
    {
        fprintf(stderr,"%s: bind: %s\n",program,strerror(errno));
        close(locale.sockfd);
        return -1;
    }
    listen(locale.sockfd,backlog);
    remote.addrlen=sizeof(struct sockaddr);
    if((creat(database,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))==-1)
    {
        fprintf(stderr,"%s: creat: %s\n",program,strerror(errno));
        close(locale.sockfd);
        return -1;
    }
    pid_t pid;
    int e=0;
    if((shmSettings(&shmid,&semid))==-1)
    {
        close(locale.sockfd);
        return -1;
    }
    printf("%s: Ascolto sulla porta %d…\n",program,ntohs(locale.addr.sin_port));
    while(true)
    {
        if((remote.sockfd=accept(locale.sockfd,(struct sockaddr*)&remote.addr,&remote.addrlen))==-1)
        {
            fprintf(stderr,"%s: accept: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        if((pid=fork())==-1)
        {
            fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        if(!pid)
        {
            close(locale.sockfd);
            if((session(remote))==-1)
            {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        close(remote.sockfd);
    }
    close(locale.sockfd);
    return e;
}

int main(int argc, char**argv)
{
    extern int errno;
    strcpy(program,basename(argv[0]));
    signal(SIGINT,signalHandler);
    if(argc!=2)
    {
        printf("usage: %s <porta>\n",program);
    }
    else
    {
        if((server(atoi(argv[1])))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}