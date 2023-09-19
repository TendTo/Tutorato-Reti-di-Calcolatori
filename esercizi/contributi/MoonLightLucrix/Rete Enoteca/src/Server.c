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
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define N 4096
#define MAX_FORM_LENGTH 32
#define backlog 10
#define tableArticolo "Articolo.txt"
#define INADDR_ANY6 "::"
typedef __uint32_t ID;
typedef struct
{
    ID id;
    char nomeAzienda[MAX_FORM_LENGTH];
    char nomeVino[MAX_FORM_LENGTH];
    unsigned int quantità;
    double costo;
} Articolo;
typedef struct
{
    int sockfd;
    struct sockaddr_in6 addr;
    socklen_t addrlen;
} SocketInfo;
char program[FILENAME_MAX];

/**
 * @brief Funzione che viene evocata al ricezione del signal interrupt evita che ci siano processi zombie in esecuzione quando si vuole terminare il programma.
 * @param signal Tipo di signal registrato sulla quale la funzione signal viene impostata sulla cattura, in questo caso il signal di tipo SIGINT. 
*/
void signalHandler(int signal)
{
    kill(0,SIGTERM);
}

/**
 * @brief Aggiunge un nuovo articolo al tabella Articolo.
 * @param articolo Tutte le informazioni dell'articolo.
 * @param fsd File descriptor stream del file dove vengono inserite le new entry.
*/
void fadd(Articolo articolo, FILE*fsd)
{
    fprintf(fsd,"%d\t%s\t%s\t%d\t%f\n",articolo.id,articolo.nomeAzienda,articolo.nomeVino,articolo.quantità,articolo.costo);
}

/**
 * @brief Trova la riga nel database in base alla chiave primaria.
 * @param key Chiave primaria da cercare, questa è univoca per ogni riga.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Viene restituito un puntatore al primo char della tupla trovata, altrimenti restituisce NULL se la tupla non esiste.
*/
char*ffind(ID key, FILE*fsd)
{
    rewind(fsd);
    ID akey;
    char line[N];
    while(fgets(line,N,fsd))
    {
        sscanf(line,"%d",&akey);
        if(!(key^akey))
        {
            return strdup(line);
        }
    }
    return NULL;
}

/**
 * @brief Conta tutte le tuple presenti nel database.
 * @param fsd File descriptor stream del file dove viene richiesto il conteggio.
 * @return Numero totale di tuple nel database.
*/
__uint32_t fcount(FILE*fsd)
{
    rewind(fsd);
    __uint32_t c=0;
    for(char line[N]; fgets(line,N,fsd); c-=-1){}
    return c;
}

/**
 * @brief Elimina la riga richiesta come parametro se questa viene trovata.
 * @param row Riga da eliminare.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
*/
void fdelete(char*row, FILE*fsd)
{
    rewind(fsd);
    char line[N];
    size_t size;
    FILE*ftmp=tmpfile();
    while(fgets(line,N,fsd))
    {
        if(strcmp(line,row))
        {
            fputs(line,ftmp);
        }
    }
    rewind(fsd);
    rewind(ftmp);
    do{
        bzero(line,strlen(line));
        size=fread(line,1,N,ftmp);
        fwrite(line,1,size,fsd);
    }while(size==N);
    ftruncate(fileno(fsd),ftell(fsd));
    fclose(ftmp);
}

/**
 * @brief Restituisce i campi di una tupla all'interno di un tipo Articolo e poi procede ad eliminare la tupla.
 * @param articolo Se la tupla viene trovata viene riempita con tutti i campi della tupla.
 * @param key Chiave primaria della tupla da cercare.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Rstituisce true se l'operazione è andata a buon fine, false altrimenti.
*/
bool fpop(Articolo*articolo, ID key, FILE*fsd)
{
    char*row=ffind(key,fsd);
    bool lineExists;
    if((lineExists=(row)))
    {
        char line[strlen(row)];
        strcpy(line,row);
        free(row);
        sscanf(line,"%d\t%[^\t]\t%[^\t]\t%d\t%lf",&articolo->id,articolo->nomeAzienda,articolo->nomeVino,&articolo->quantità,&articolo->costo);
        fdelete(line,fsd);        
    }
    return lineExists;
}

//connection process

/**
 * @brief Invia al Client la lista completa degli articoli.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce -1 ed un messaggio di errno se si è verificato un problema, 0 altrimenti.
*/
int lista(SocketInfo remote)
{
    char buffer[N];
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"r")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    dprintf(remote.sockfd,"%s: Lista vini:\n",program);
    while(fgets(buffer,N,fsd))
    {
        if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
        {
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            return -1;
        }
    }
    dprintf(remote.sockfd,"\n\n");
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Permette al Client di acquistare un vino se la quantità richiesta dal Client è minore di quella presenta nel Database, se no manda un messaggio di errore al Client.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce 0 se l'operazione è andata a buon fine, -1 altrimenti.
*/
int acquisto(SocketInfo remote)
{
    Articolo articolo;
    char buffer[N];
    ssize_t size;
    ID id;
    dprintf(remote.sockfd,"%s: Inserire ID articolo: ",program);
    bzero(buffer,strlen(buffer));
    if((size=read(remote.sockfd,buffer,N))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        return -1;
    }
    buffer[size]='\0';
    id=atoi(buffer);
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    if(fpop(&articolo,id,fsd))
    {
        unsigned int quantità;
        dprintf(remote.sockfd,"\n\nVino: %s\nazienda: %s\nquantità: %d\ncosto: €%f\n\n%s: Inserire la quantità da comprare: ",articolo.nomeVino,articolo.nomeAzienda,articolo.quantità,articolo.costo,program);
        bzero(buffer,strlen(buffer));
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            return -1;
        }
        buffer[size]='\0';
        quantità=atoi(buffer);
        if((articolo.quantità>=quantità))
        {
            articolo.quantità-=quantità;
            dprintf(remote.sockfd,"%s: Acquisto effettuato!\n",program);
        }
        else
        {
            dprintf(remote.sockfd,"%s: Quantità insufficiente!\n",program);
        }
        fadd(articolo,fsd);
    }
    else
    {
        dprintf(remote.sockfd,"%s: Articolo non trovato!\n",program);
    }
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Gestisce la connessione col Client.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int connection(SocketInfo remote)
{
    char buffer[N];
    ssize_t size;
    dprintf(remote.sockfd,"\n\n  __                              __            _                   _ \n / _\\ ___ _ ____   _____ _ __    /__\\ __   ___ | |_ ___  ___ __ _  / \\\n \\ \\ / _ \\ '__\\ \\ / / _ \\ '__|  /_\\| '_ \\ / _ \\| __/ _ \\/ __/ _` |/  /\n _\\ \\  __/ |   \\ V /  __/ |    //__| | | | (_) | ||  __/ (_| (_| /\\_/ \n \\__/\\___|_|    \\_/ \\___|_|    \\__/|_| |_|\\___/ \\__\\___|\\___\\__,_\\/   \n\n\n\n\n");
    int e=0;
    while(true)
    {
        dprintf(remote.sockfd,"+----------------+\n| l - lista vini |\n| a - acquista   |\n| q - esci       |\n+----------------+\n\n\n\n%s: scelta: ",program);
        bzero(buffer,strlen(buffer));
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if(!strcmp(buffer,"l"))
        {
            e=lista(remote);
        }
        else if(!strcmp(buffer,"a"))
        {
            e=acquisto(remote);
        }
        else if(!strcmp(buffer,"q"))
        {
            dprintf(remote.sockfd,"%s: Arrivederci!\n",program);
            break;
        }
        else if(!strcmp(buffer,"\n"))
        {
            continue;
        }
        else
        {
            dprintf(remote.sockfd,"%s: Comando sconosciuto\n",program);
        }
        if(e==-1)
        {
            break;
        }
    }
    close(remote.sockfd);
    return e;
}

//connectionAzienda process

/**
 * @brief Invia al Client-Azienda la lista completa dei suoi articoli.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce -1 ed un messaggio di errno se si è verificato un problema, 0 altrimenti.
*/
int lista2(SocketInfo remote, char*nomeAzienda)
{
    char buffer[N];
    ssize_t size;
    FILE*fsd;
    char anomeAzienda[MAX_FORM_LENGTH];
    if(!(fsd=fopen(tableArticolo,"r")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    dprintf(remote.sockfd,"%s: Lista vini:\n",program);
    flock(fileno(fsd),LOCK_EX);
    while(fgets(buffer,N,fsd))
    {
        sscanf(buffer,"%*[^\t]\t%[^\t]",anomeAzienda);
        if(!strcmp(anomeAzienda,nomeAzienda))
        {
            if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
            {
                flock(fileno(fsd),LOCK_UN);
                fclose(fsd);
                fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
                return -1;
            }
        }
    }
    dprintf(remote.sockfd,"\n\n");
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Permette al Client-Azienda di inserire nuove tuple all'interno del database.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int aggiungi(SocketInfo remote)
{
    Articolo articolo;
    if((read(remote.sockfd,&articolo,sizeof(Articolo)))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        return -1;
    }
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    articolo.id=fcount(fsd);
    fadd(articolo,fsd);
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Permette al Client-Azienda di modificare la quantità di un articolo giù presente nel database.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param nomeAzienda Nome dell'azienda che sta effettuando la modifica.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int aggiorna(SocketInfo remote, char*nomeAzienda)
{
    Articolo articolo;
    char buffer[N];
    ssize_t size;
    ID id;
    dprintf(remote.sockfd,"%s: Inserisci ID del vino da aggiornare: ",program);
    bzero(buffer,strlen(buffer));
    if((size=read(remote.sockfd,buffer,N))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        return -1;
    }
    buffer[size]='\0';
    id=atoi(buffer);
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"r+")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    if(fpop(&articolo,id,fsd))
    {
        if(!strcmp(nomeAzienda,articolo.nomeAzienda))
        {
            dprintf(remote.sockfd,"\n\nVino: %s\nazienda: %s\nquantità: %d\ncosto: €%f\n\n%s: Inserire la nuova quantità: ",articolo.nomeVino,articolo.nomeAzienda,articolo.quantità,articolo.costo,program);
            bzero(buffer,strlen(buffer));
            if((size=read(remote.sockfd,buffer,N))==-1)
            {
                flock(fileno(fsd),LOCK_UN);
                fclose(fsd);
                fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
                return -1;
            }
            buffer[size]='\0';
            articolo.quantità=atoi(buffer);
        }
        else
        {
            dprintf(remote.sockfd,"%s: Impossibile modificare la quantità\n",program);
        }
        fadd(articolo,fsd);
    }
    else
    {
        dprintf(remote.sockfd,"%s: Articolo non trovato!\n",program);
    }
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Gestisce la connessione col Client-Azienda.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param nomeAzienda Indentifica l'azienda con cui si sta effettuando la comunicazione.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int connectionAzienda(SocketInfo remote, char*nomeAzienda)
{
    char buffer[N];
    ssize_t size;
    int e=0;
    dprintf(remote.sockfd,"\n\n  __                              __            _                   _ \n / _\\ ___ _ ____   _____ _ __    /__\\ __   ___ | |_ ___  ___ __ _  / \\\n \\ \\ / _ \\ '__\\ \\ / / _ \\ '__|  /_\\| '_ \\ / _ \\| __/ _ \\/ __/ _` |/  /\n _\\ \\  __/ |   \\ V /  __/ |    //__| | | | (_) | ||  __/ (_| (_| /\\_/ \n \\__/\\___|_|    \\_/ \\___|_|    \\__/|_| |_|\\___/ \\__\\___|\\___\\__,_\\/   \nAzienda Vers.\n\n\n\n");
    while(true)
    {
        dprintf(remote.sockfd,"+-----------------------+\n| l - lista dei vini    |\n| a - aggiungi vino     |\n| u - aggiorna quantità |\n| q - esci              |\n+-----------------------+\n\n\n\n\n\n\n%s: Scelta: ",program);
        bzero(buffer,strlen(buffer));
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if(!strcmp(buffer,"l"))
        {
            e=lista2(remote,nomeAzienda);
        }
        else if(!strcmp(buffer,"a"))
        {
            e=aggiungi(remote);
        }
        else if(!strcmp(buffer,"u"))
        {
            e=aggiorna(remote,nomeAzienda);
        }
        else if(!strcmp(buffer,"q"))
        {
            dprintf(remote.sockfd,"%s: Arrivederci!\n",program);
            break;
        }
        else if(!strcmp(buffer,"\n"))
        {
            continue;
        }
        else
        {
            dprintf(remote.sockfd,"%s: Comando sconosciuto\n",program);
        }
        if(e==-1)
        {
            break;
        }
    }
    close(remote.sockfd);
    return e;
}

//Parent process

/**
 * @brief Setta tutti le varie impostazioni per la socket e la struct sockaddr_in6.
 * @param sockaddr SocketInfo da settare.
 * @param address Indirizzo IPv6 a cui mandare pacchetti, se viene passata come NULL verrà impostata a in6addr_any.
 * @param porta Porta sulla quale indirizzare i pacchetti entranti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int sockaddr6Settings(SocketInfo*sockaddr, char*address, in_port_t porta)
{
    sockaddr->addrlen=sizeof(struct sockaddr_in6);
    if((sockaddr->sockfd=socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP))==-1)
    {
        fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
        return -1;
    }
    memset((char*)&sockaddr->addr,0,sockaddr->addrlen);
    sockaddr->addr.sin6_family=AF_INET6;
    //inet_pton(AF_INET6,((!address)?INADDR_ANY6:address),&sockaddr->addr.sin6_addr);
    sockaddr->addr.sin6_addr=in6addr_any;
    sockaddr->addr.sin6_port=htons(porta);
    return 0;
}

/**
 * @brief Funzione che viene utilizzata come server nella comunicazione, attende che sia effettuata l'accept per creare una connessione che continuerà la comunicazione col client, mentre il processo padre si rimette in ascolto, il protocollo utilizzato è Trasmission Control Protocol.
 * @param porta Porta sulla quale vengono reindirizzati i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int server(in_port_t porta)
{
    SocketInfo locale, remote;
    if((sockaddr6Settings(&locale,NULL,porta))==-1)
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
    remote.addrlen=sizeof(struct sockaddr_in6);
    if((creat(tableArticolo,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))==-1)
    {
        fprintf(stderr,"%s: creat: %s\n",program,strerror(errno));
        close(locale.sockfd);
        return -1;
    }
    pid_t pid;
    char nomeAzienda[MAX_FORM_LENGTH];
    ssize_t size;
    int e=0;
    printf("%s: Attendo connessioni sulla porta %d...\n",program,ntohs(locale.addr.sin6_port));
    while(true)
    {
        if((remote.sockfd=accept(locale.sockfd,(struct sockaddr*)&remote.addr,&remote.addrlen))==-1)
        {
            fprintf(stderr,"%s: accept: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        bzero(nomeAzienda,strlen(nomeAzienda));
        if((size=read(remote.sockfd,nomeAzienda,MAX_FORM_LENGTH))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            return -1;
        }
        nomeAzienda[size]='\0';
        if((pid=fork())==-1)
        {
            fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
            close(remote.sockfd);
            e=-1;
            break;
        }
        if(!pid)
        {
            close(locale.sockfd);
            if(!strcmp(nomeAzienda,"\n"))
            {
                if((connection(remote))==-1)
                {
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                if((connectionAzienda(remote,nomeAzienda))==-1)
                {
                    exit(EXIT_FAILURE);
                }
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