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
#include<time.h>
#include<errno.h>
#define N 4096
#define S_MUTEX 0
#define S_COND_1 1
#define MAX_FORM_LENGTH 32
#define MAX_MSG_SIZE 20
#define online 'x'
#define offline 'o'
#define forwarded 'f'
#define unforwarded 'u'
#define database "lista.txt"
#define backlog 6
typedef enum{C,Cpp,Java,Python,R,Matlab} ID;
typedef struct
{
    char nome[MAX_FORM_LENGTH];
    ID id;
    char address[INET_ADDRSTRLEN];
    __uint16_t porta;
    bool status;
    bool forward;
} Linguaggio;
typedef struct
{
    Linguaggio linguaggio;
    ID idList[backlog];
    int numeroUtenti;
    int numeroUtentiRimasti;
    char msg[MAX_MSG_SIZE];
} Messaggio;
typedef struct
{
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
} SocketInfo;
char program[FILENAME_MAX];
int shmid, semid;

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
 * @brief Setta tutti le varie impostazioni per la socket e la struct sockaddr_in.
 * @param sockaddr SocketInfo da settare.
 * @param address Indirizzo IPv4 a cui mandare pacchetti, se viene passata come NULL verrà impostata a INADDR_ANY.
 * @param porta Porta sulla quale indirizzare i pacchetti entranti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int sockaddrSettings(SocketInfo*sockaddr, char*address, __uint16_t porta)
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
 * @brief Aggiunge un nuovo utente nel database.
 * @param linguaggio Tutte le informazioni del linguaggio.
 * @param fsd File descriptor stream del file dove vengono inserite le new entry.
*/
void fadd(Linguaggio linguaggio, FILE*fsd)
{
    fprintf(fsd,"%s\t%d\t%s\t%d\t%c\t%c\n",linguaggio.nome,linguaggio.id,linguaggio.address,linguaggio.porta,(linguaggio.status)?online:offline,(linguaggio.forward)?forwarded:unforwarded);
}

/**
 * @brief Trova la riga nel database in base alla prima chiave primaria.
 * @param key Chiave primaria da cercare, questa è univoca per ogni riga.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Viene restituito un puntatore al primo char della riga trovata, altrimenti restituisce NULL se la riga non esiste.
*/
char*ffind(char*key, FILE*fsd)
{
    rewind(fsd);
    char buffer[N];
    char*line;
    char akey[strlen(key)];
    while((line=fgets(buffer,N,fsd)))
    {
        sscanf(line,"%[^\t]",akey);
        if(!strcmp(key,akey))
        {
            return strdup(line);
        }
    }
    return NULL;
}

/**
 * @brief Trova la riga nel database in base alla seconda chiave primaria.
 * @param id Chiave primaria da cercare, questa è univoca per ogni riga.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Viene restituito un puntatore al primo char della riga trovata, altrimenti restituisce NULL se la riga non esiste.
*/
char*ffindById(ID id, FILE*fsd)
{
    rewind(fsd);
    char buffer[N];
    char*line;
    ID aid;
    while((line=fgets(buffer,N,fsd)))
    {
        sscanf(line,"%*[^\t]\t%d",&aid);
        if(!(id^aid))
        {
            return strdup(line);
        }
    }
    return NULL;
}

/**
 * @brief Elimina la linea richiesta come parametro se questa viene trovata.
 * @param line Linea da eliminare
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
*/
void fdelete(char*line, FILE*fsd)
{
    rewind(fsd);
    FILE*ftmp=tmpfile();
    char aline[N];
    size_t size;
    while(fgets(aline,N,fsd))
    {
        if(strcmp(line,aline))
        {
            fputs(aline,ftmp);
        }
    }
    rewind(fsd);
    rewind(ftmp);
    do{
        memset(aline,'\0',strlen(aline));
        size=fread(aline,1,N,ftmp);
        fwrite(aline,1,size,fsd);
    }while(N==size);
    fclose(ftmp);
    ftruncate(fileno(fsd),ftell(fsd));
    /*rename(ftmp,fsd);
    fseek(fsd,SEEK_END,0);*/
}

/**
 * @brief Inserisce tutti i campi della linea all'interno di linguaggio.
 * @param line Linea dove prendere le informazioni.
 * @param fsd File descriptor stream del file usato come database.
 * @return Un tipo linguaggio in cui sono presenti tutte le informazioni della tupla.
*/
Linguaggio fpop(char*line, FILE*fsd)
{
    Linguaggio linguaggio;
    char status, forward;
    sscanf(line,"%[^\t]\t%d\t%[^\t]\t%hd\t%c\t%c",linguaggio.nome,&linguaggio.id,linguaggio.address,&linguaggio.porta,&status,&forward);
    linguaggio.status=(status==online);
    linguaggio.forward=(forward==forwarded);
    return linguaggio;
}

/**
 * @brief Setta tutti i campi "forward" all'interno del database a false.
 * @param fsd File descriptor stream del file dove vengono settati i campi.
*/
void fset(FILE*fsd)
{
    rewind(fsd);
    FILE*ftmp=tmpfile();
    char line[N];
    size_t size;
    while(fgets(line,N,fsd))
    {
        Linguaggio linguaggio=fpop(line,fsd);
        linguaggio.forward=false;
        fadd(linguaggio,ftmp);
    }
    rewind(fsd);
    rewind(ftmp);
    do{
        memset(line,'\0',strlen(line));
        size=fread(line,1,N,ftmp);
        fwrite(line,1,size,fsd);
    }while(N==size);
    fclose(ftmp);
    ftruncate(fileno(fsd),ftell(fsd));
    //rename(ftmp,fsd);
}

/**
 * @brief Conta tutti i linguaggi online del database.
 * @param fsd File descriptor stream del file dove vengono contate le entry.
 * @return Numero complessivo di linguaggi online nel database come intero.
*/
int fcount(ID*idList, int n, FILE*fsd)
{
    int numeroUtenti=n;
    for(int i=0; i<=n-1; i-=-1)
    {
        char*lineExists=ffindById(idList[i],fsd);
        if(lineExists)
        {
            char line[strlen(lineExists)];
            strcpy(line,lineExists);
            free(lineExists);
            Linguaggio alinguaggio=fpop(line,fsd);
            numeroUtenti-=(!alinguaggio.status)?1:0;
        }
    }
    return numeroUtenti;
}

/**
 * @brief Algoritmo di ordinamento usato per ordinare i linguaggi più vicini in modo tale da sistemare gli ID della riga della tabella in maniera corretta per l'invio.
 * @param vett1 Vettore di interi senza segno a 8 bit che contiene tutti i valori della tupla in questione.
 * @param vett2 Vettori di ID della tupla sulla quale viene fatto l'ordinamento.
 * @param sx Indice sinistro dei vettori usato per l'algoritmo del quicksort.
 * @param dx Indice destro dei vettori usato per l'algoritmo del quicksort.
*/
void quicksort(__uint8_t*vett1, ID*vett2, int sx, int dx)
{
    int i=sx, j=dx;
    __uint8_t pivot=vett1[(sx+dx)>>1], a;
    ID b;
    while(i<=j)
    {
        while(vett1[i]<pivot)
        {
            i-=-1;
        }
        while(vett1[j]>pivot)
        {
            j--;
        }
        if(i<=j)
        {
            a=vett1[i];
            vett1[i]=vett1[j];
            vett1[j]=a;
            b=vett2[i];
            vett2[i]=vett2[j];
            vett2[j]=b;
            i-=-1;
            j--;
        }
    }
    if(sx<=j)
    {
        quicksort(vett1,vett2,sx,j);
    }
    if(i<=dx)
    {
        quicksort(vett1,vett2,i,dx);
    }
}

//ReceiverFOSs process -> session process

/**
 * @brief Controlla se l'ID si trova nella lista degli ID riceventi.
 * @param idList vettore contenente gli ID dei riceventi.
 * @param id ID passato come parametro per la verifica.
 * @param n Numero di linguaggi a cui recapitare il messaggio.
 * @return TRUE se l'ID si trova nella lista, FALSE altrimenti.
*/
bool isInList(ID*idList, ID id, int n)
{
    for(int i=0; i<=n-1; i-=-1)
    {
        if(idList[i]==id)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Questa funzione viene evocata dalla sessione dedicata col client prima che si metta in ascolto dei comandi mandati dal client, si mette in attesa di un messaggio inviato da un altra sessione per poterlo inoltrare al client in collegamento sulla sessione corrente.
 * @param remote Struct contenente tutte le informazioni legate alla socket.
 * @param linguaggio Struct contenente tutte le informazioni del linguaggio che durante la funzione vengono modificate e registrate sul database.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int receiverFromOtherSessions(SocketInfo remote, Linguaggio linguaggio)
{
    char buffer[N];
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
        if(!(fsd=fopen(database,"r+")))
        {
            fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
            messaggio->numeroUtentiRimasti=0;
            SIGNAL(semid,S_MUTEX);
            continue;
        }
        flock(fileno(fsd),LOCK_EX);
        char*lineExists=ffind(linguaggio.nome,fsd);
        if(lineExists)
        {
            char line[strlen(lineExists)];
            strcpy(line,lineExists);
            free(lineExists);
            linguaggio=fpop(line,fsd);
            //printf("Sono %s\n",linguaggio.nome);
            if((linguaggio.status) && (!linguaggio.forward) && (isInList(messaggio->idList,linguaggio.id,messaggio->numeroUtenti)))
            {
                //printf("Mando il messaggio a: %s\n",linguaggio.nome);
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"\n%s: %s\n%s: Riprendi da dove avevi lasciato: ",messaggio->linguaggio.nome,messaggio->msg,program);
                if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
                {
                    fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
                    messaggio->numeroUtentiRimasti=0;
                    flock(fileno(fsd),LOCK_UN);
                    fclose(fsd);
                    SIGNAL(semid,S_MUTEX);
                    continue;
                }
                fdelete(line,fsd);
                linguaggio.forward=true;
                fadd(linguaggio,fsd);
                messaggio->numeroUtentiRimasti--;
                //printf("Utenti rimasti: %d\n",messaggio->numeroUtentiRimasti);
            }
        }
        flock(fileno(fsd),LOCK_UN);
        fclose(fsd);
        if(messaggio->numeroUtentiRimasti>=1)
        {
            //printf("ciao\n");
            SIGNAL(semid,S_COND_1);
        }
        else
        {
            SIGNAL(semid,S_MUTEX);
        }
    }
    return 0;
}

//Session process

/**
 * @brief Si occupa di inizializzare la struct condivisa con i campi struct del linguaggio che ha generato il messaggio, messaggio, il numero di utenti rimasti connessi a cui mandare il messaggio ed infine risveglia il processo ReceiverFOS.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param n Numero di linguaggi a cui mandare il messaggio.
 * @param msg Messaggio mandato dal client da inoltrare alle altre sessioni in modo tale che a loro volta lo inoltrino agli altri client connessi.
 * @param linguaggio Struct che viene utilizzata per inizializzare la struct condivisa tra le sessioni con le varie informazioni, ad esempio, il linguaggio che ha generato il messaggio.
 * @param fsd File descriptor stream che viene utilizzato come database.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int forwardMessage(SocketInfo remote, int n, char*msg, Linguaggio linguaggio, __uint8_t distanze[backlog][backlog])
{
    Messaggio*messaggio;
    __uint8_t rigaDistanze[backlog];
    ID idList[backlog]={C,Cpp,Java,Python,R,Matlab};
    FILE*fsd;
    for(int i=0; i<=backlog-1; i-=-1)
    {
        if(idList[i]==linguaggio.id)
        {
            memcpy(rigaDistanze,distanze+i,sizeof(__uint8_t)*backlog);
        }
    }
    quicksort(rigaDistanze,idList,0,backlog-1);
    if((messaggio=(Messaggio*)shmat(shmid,NULL,0))==(Messaggio*)-1)
    {
        fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
        return -1;
    }
    WAIT(semid,S_MUTEX);
    messaggio->linguaggio=linguaggio;
    memcpy(messaggio->idList,idList,sizeof(ID)*backlog);
    strcpy(messaggio->msg,msg);
    if(!(fsd=fopen(database,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        SIGNAL(semid,S_MUTEX);
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    fset(fsd);
    messaggio->numeroUtenti=messaggio->numeroUtentiRimasti=fcount(idList,n,fsd);
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    /*printf("Riga distanza ordinata: ");
    for(int i=0; i<=n-1; i-=-1)
    {
        printf("%d ",rigaDistanze[i]);
    }
    printf("\n");
    printf("Devo mandare il messaggio a: ");
    for(int i=0; i<=n-1; i-=-1)
    {
        printf("%d ",messaggio->idList[i]);
    }
    printf("\n");
    printf("NumeroUtenti: %d\n",messaggio->numeroUtenti);*/
    SIGNAL(semid,S_COND_1);
    return 0;
}

/**
 * @brief Funzione che si occupa dell'accesso del linguaggio, non permette che due utenti siano connessi nello stesso momento.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param linguaggio In caso di successo conterrà tutte le informazioni del linguaggio.
 * @param nome Nome del linguaggio da verificare nel database.
 * @param fsd File descriptor stream del file usato come database.
 * @return Restituisce -1 se un altro utente con lo stesso username è già connesso, 0 se la registrazione o accesso ha avuto successo.
*/
int signIn(SocketInfo remote, Linguaggio*linguaggio, char*nome, FILE*fsd)
{
    char*lineExists=ffind(nome,fsd);
    if(lineExists)
    {
        char line[strlen(lineExists)];
        strcpy(line,lineExists);
        free(lineExists);
        *linguaggio=fpop(line,fsd);
        if(linguaggio->status)
        {
            dprintf(remote.sockfd,"%s: Il linguaggio è già connesso\n",program);
            return -1;
        }
        fdelete(line,fsd);
        strcpy(linguaggio->address,inet_ntoa(remote.addr.sin_addr));
        linguaggio->porta=ntohs(remote.addr.sin_port);
        linguaggio->status=true;
        linguaggio->forward=false;
        fadd(*linguaggio,fsd);
        dprintf(remote.sockfd,"%s: Accesso eseguito\n",program);
        return 0;
    }
    dprintf(remote.sockfd,"%s: Linguaggio sconosciuto\n",program);
    return -1;
}

/**
 * @brief Effettua il log-out del linguaggio dalla sessione corrente aggiornando il database che provederà a settare a offline il record.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param lunguaggio Struct contenente tutte le informazioni del linguaggio con la quale aggiornare il database.
 * @param fsd File descriptor stream del file usato come database.
*/
void signOut(SocketInfo remote, Linguaggio*linguaggio, FILE*fsd)
{
    char*lineExists=ffind(linguaggio->nome,fsd);
    if(lineExists)
    {
        char line[strlen(lineExists)];
        strcpy(line,lineExists);
        free(lineExists);
        fdelete(line,fsd);
        linguaggio->status=false;
        fadd(*linguaggio,fsd);
        dprintf(remote.sockfd,"%s: Disconnesso\n",program);
    }
}

/**
 * @brief Gestisce la sessione col linguaggio, se un operazione torna -1 il server slogga l'utente e termina il processo sessione, crea un processo che si mette in ascolto di messaggi provenienti da altre sessioni del server con altri client per poter inoltrare messaggi provenienti da altri linguaggi.
 * @param remote Struct contenete tutte le informazioni della nuova socket di sessione con il client.
 * @param distanze Matrice di distanze composta da interi senza segno a 8 bit.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int session(SocketInfo remote, __uint8_t distanze[backlog][backlog])
{
    Linguaggio linguaggio;
    char buffer[N];
    ssize_t size;
    FILE*fsd;
    struct timeval timeout={10,0};
    if((setsockopt(remote.sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(struct timeval)))==-1)
    {
        fprintf(stderr,"%s: setsockopt: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    memset(buffer,'\0',strlen(buffer));
    if((size=read(remote.sockfd,buffer,N))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    buffer[size]='\0';
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
    if((signIn(remote,&linguaggio,buffer,fsd))==-1)
    {
        close(remote.sockfd);
        return 0;
    }
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    pid_t pid;
    if((pid=fork())==-1)
    {
        fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if(!pid)
    {
        if((receiverFromOtherSessions(remote,linguaggio))==-1)
        {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    int e=0, n=0;
    dprintf(remote.sockfd,"\x1b[1;36m   ________  _____  _______    ____  ___\n  / ____/ / / /   |/_  __/ |  / /  |/  /\n / /   / /_/ / /| | / /  | | / / /|_/ / \n/ /___/ __  / ___ |/ /   | |/ / /  / /  \n\\____/_/ /_/_/  |_/_/    |___/_/  /_/\x1b[0m   \n\n\n\n\n\n\n\n\n");
    while(true)
    {
        while(true)
        {
            dprintf(remote.sockfd,"%s: Inserisci un valore compreso tra 1 e 6: ",program);
            memset(buffer,'\0',strlen(buffer));
            if((size=read(remote.sockfd,buffer,N))==-1)
            {
                fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
                e=-1;
                break;
            }
            buffer[size]='\0';
            if(!strcmp(buffer,"quit"))
            {
                break;
            }
            n=atoi(buffer);
            if((1<=n) && (n<=6))
            {
                break;
            }
            dprintf(remote.sockfd,"%s: \x1b[31mIl numero inserito non è compreso tra 1 e 6\x1b[0m\n",program);
        }
        if((!strcmp(buffer,"quit")) || (e==-1))
        {
            break;
        }
        dprintf(remote.sockfd,"%s: Inserisci il messaggio da inviare: ",program);
        memset(buffer,'\0',strlen(buffer));
        if((size=read(remote.sockfd,buffer,MAX_MSG_SIZE))==-1)
        {
            fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if((forwardMessage(remote,n,buffer,linguaggio,distanze))==-1)
        {
            e=-1;
            break;
        }
    }
    if(!(fsd=fopen(database,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        kill(pid,SIGTERM);
        close(remote.sockfd);
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    signOut(remote,&linguaggio,fsd);
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    kill(pid,SIGTERM);
    close(remote.sockfd);
    return e;
}

//Parent process

/**
 * @brief Crea il database e gli inserisce già tutte le informazioni nulle, tranne per il nome dei linguaggi.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int creaDatabase()
{
    FILE*fsd;
    if(!(fsd=fopen(database,"w+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    char nome[backlog][MAX_FORM_LENGTH]={"C","C++","Java","Python","R","Matlab"};
    ID id[backlog]={C,Cpp,Java,Python,R,Matlab};
    for(int i=0; i<=backlog-1; i-=-1)
    {
        Linguaggio linguaggio;
        strcpy(linguaggio.nome,nome[i]);
        linguaggio.id=id[i];
        strcpy(linguaggio.address,"0.0.0.0");
        linguaggio.porta=0;
        linguaggio.status=false;
        linguaggio.forward=false;
        fadd(linguaggio,fsd);
    }
    fclose(fsd);
    return 0;
}

/**
 * @brief Popola la matrice delle distanze in modo tale che la diagonale principale abbia tutti 0 e che le altre siano specchiate.
 * @param distanze Matrice di interi senza segno a 8 bit che indicano le distanze.
*/
void popolaMatrice(__uint8_t distanze[backlog][backlog])
{
    srand(time(NULL));
    for(int i=0; i<=backlog-1; i-=-1)
    {
        for(int j=0; j<=i; j-=-1)
        {
            distanze[i][j]=distanze[j][i]=(i==j)?0:(rand()%245)+10;
        }
    }
}

/**
 * @brief Stampa la matrice.
 * @param distanze Matrice di interi senza segno a 8 bit che indicano le distanze.
*/
void stampaMatrice(__uint8_t distanze[backlog][backlog])
{
    char nome[backlog][MAX_FORM_LENGTH]={"C","C++","Java","Python","R","Matlab"};
    for(int i=-1; i<=backlog-1; i-=-1)
    {
        if(i>=0)
        {
            if(!(i%2))
            {
                printf("\x1b[1;102;94m%s\t\x1b[0m",nome[i]);
            }
            else
            {
                printf("\x1b[1;42;94m%s\t\x1b[0m",nome[i]);
            }
        }
        else
        {
            printf("\x1b[1;106;35mD\t\x1b[0m");
        }
        for(int j=0; j<=backlog-1; j-=-1)
        {
            if(i>=0)
            {
                if(i%2)
                {
                    printf("\x1b[107m");
                }
                if(i==j)
                {
                    printf("\x1b[1;32m%d\t\x1b[0m",distanze[i][j]);
                }
                else
                {
                    printf("\x1b[34m%d\t\x1b[0m",distanze[i][j]);
                }
            }
            else
            {
                if(!(j%2))
                {
                    printf("\x1b[1;102;94m%s\t\x1b[0m",nome[j]);
                }
                else
                {
                    printf("\x1b[1;42;94m%s\t\x1b[0m",nome[j]);
                }
            }
        }
        printf("\n");
    }
}

/**
 * @brief Funzione che viene utilizzata come server nella comunicazione, attende che sia effettuata l'accept per creare una sessione che continuerà la comunicazione col client, mentre il processo padre si rimette in ascolto, il protocollo utilizzato è Trasmission Control Protocol.
 * @param porta Porta sulla quale vengono reindirizzati i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int server(__uint16_t porta)
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
    pid_t pid;
    if((creaDatabase())==-1)
    {
        close(locale.sockfd);
        return -1;
    }
    __uint8_t distanze[backlog][backlog];
    popolaMatrice(distanze);
    if((shmSettings(&shmid,&semid))==-1)
    {
        close(locale.sockfd);
        return -1;
    }
    remote.addrlen=sizeof(struct sockaddr);
    int e=0;
    printf("\x1b[32m%s: Attendo connessioni sulla porta %hd…\x1b[0m\n",program,ntohs(locale.addr.sin_port));
    stampaMatrice(distanze);
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
            if((session(remote,distanze))==-1)
            {
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        close(remote.sockfd);
    }
    close(locale.sockfd);
    shmctl(shmid,IPC_RMID,NULL);
    semctl(semid,0,IPC_RMID,0);
    return e;
}

int main(int argc, char**argv)
{
    extern int errno;
    strcpy(program,basename(argv[0]));
    signal(SIGINT,signalHandler);
    if(argc!=2)
    {
        printf("\x1b[31musage: %s <porta>\x1b[0m\n",program);
    }
    else
    {
        if(server(atoi(argv[1]))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}