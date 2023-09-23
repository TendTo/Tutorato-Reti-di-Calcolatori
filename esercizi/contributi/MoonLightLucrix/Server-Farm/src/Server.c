/**
 * @author MoonLightLucrix
 * @link https://github.com/MoonLightLucrix
*/
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/file.h>
#include<libgen.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define N 4096
#define MAX_FORM_LENGTH 32
#define tableArticolo "Articolo.txt"
#define tableSpedizioni "Spedizioni.txt"
#define IN6ADDR_ANY "::"
#define backlog 10
typedef __uint32_t ID;
typedef struct
{
    ID id;
    char nomeFarmaco[MAX_FORM_LENGTH];
    __uint32_t quantità;
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
void faddArticolo(Articolo articolo, FILE*fsd)
{
    fprintf(fsd,"%d\t%s\t%d\n",articolo.id,articolo.nomeFarmaco,articolo.quantità);
}

/**
 * @brief Aggiunge un nuovo articolo al tabella Spedizioni.
 * @param indirizzo Indirizzo specificato dal Cliente Remoto.
 * @param nomeFarmaco Nome del farmaco che il Cliente Remoto vuole acquistare.
 * @param quantità Quantità del farmaco che il Cliente Remoto vuole acquistare.
 * @param fsd File descriptor stream del file dove vengono inserite le new entry.
*/
void faddSpedizioni(char*indirizzo, char*nomeFarmaco, __uint32_t quantità, FILE*fsd)
{
    fprintf(fsd,"%s\t%s\t%d\n",indirizzo,nomeFarmaco,quantità);
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
    char line[N];
    ID akey;
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
 * @brief Elimina la riga richiesta come parametro se questa viene trovata.
 * @param row Riga da eliminare.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
*/
void fdelete(char*line, FILE*fsd)
{
    rewind(fsd);
    char aline[N];
    size_t size;
    FILE*ftmp=tmpfile();
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
        bzero(aline,strlen(aline));
        size=fread(aline,1,N,ftmp);
        fwrite(aline,1,size,fsd);
    }while(size==N);
    ftruncate(fileno(fsd),ftell(fsd));
    fclose(ftmp);
}

/**
 * @brief Restituisce i campi di una tupla all'interno di un tipo Articolo e poi procede ad eliminare la tupla.
 * @param articolo Se la tupla viene trovata la struct viene riempita con tutti i campi della tupla.
 * @param key Chiave primaria della tupla da cercare.
 * @param fsd File descriptor stream del file dove viene richiesta la ricerca.
 * @return Restituisce true se l'operazione è andata a buon fine, false altrimenti.
*/
bool fpop(Articolo*articolo, ID key, FILE*fsd)
{
    char*row;
    bool found=false;
    if((found=(row=ffind(key,fsd))))
    {
        char line[strlen(row)];
        strcpy(line,row);
        free(row);
        sscanf(line,"%d\t%[^\t]\t%d",&articolo->id,articolo->nomeFarmaco,&articolo->quantità);
        fdelete(line,fsd);
    }
    return found;
}

/**
 * @brief Setta tutti le varie impostazioni per la socket e la struct sockaddr_in6.
 * @param sockaddr SocketInfo da settare.
 * @param address Indirizzo IPv6 a cui mandare pacchetti, se viene passata come NULL verrà impostata a in6addr_any.
 * @param porta Porta sulla quale indirizzare i pacchetti entranti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int sockaddr6Settings(SocketInfo*remote, char*address, in_port_t porta)
{
    remote->addrlen=sizeof(struct sockaddr_in6);
    if((remote->sockfd=socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP))==-1)
    {
        fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
        return -1;
    }
    bzero((char*)&remote->addr,remote->addrlen);
    remote->addr.sin6_family=AF_INET6;
    inet_pton(AF_INET6,((!address)?IN6ADDR_ANY:address),&remote->addr.sin6_addr);
    remote->addr.sin6_port=htons(porta);
    return 0;
}

/**
 * @brief Invia al Client la lista completa degli articoli con quantità maggiore di 0.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Restituisce -1 ed un messaggio di errno se si è verificato un problema, 0 altrimenti.
*/
int lista(SocketInfo remote)
{
    char line[N];
    ssize_t size;
    __uint32_t quantità;
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"rb")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    while(fgets(line,N,fsd))
    {
        sscanf(line,"%*[^\t]\t%*[^\t]\t%d",&quantità);
        if(!quantità)
        {
            continue;
        }
        if((write(remote.sockfd,line,strlen(line)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            return -1;
        }
    }
    flock(fileno(fsd),LOCK_UN);
    fclose(fsd);
    return 0;
}

/**
 * @brief Permette al client di acquistare un farmaco se la quantità richiesta dal client è minore di quella presenta nel Database, se no manda un messaggio di errore al client, per i Client Remoti aggiunge una tupla nel Database delle spedizioni.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param id ID del farmaco che il client richiedono l'acquisto, nel caso in cui il farmaco non esiste verrà mandato al client un messaggio di errore.
 * @param indirizzo Nel caso di Client Remoto sarà una stringa non vuota che permette l'inserimento dell'indirizzo nel database delle spedizioni.
 * @return Restituisce 0 se l'operazione è andata a buon fine, -1 altrimenti.
*/
int acquisto(SocketInfo remote, ID id, char*indirizzo)
{
    Articolo articolo;
    char buffer[N];
    ssize_t size;
    FILE*fsd;
    if(!(fsd=fopen(tableArticolo,"r+b")))
    {
        fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
        return -1;
    }
    flock(fileno(fsd),LOCK_EX);
    if((fpop(&articolo,id,fsd)))
    {
        dprintf(remote.sockfd,"%s: Articolo:\nID: %d\nNome farmaco: %s\nQuantità: %d\n%s: Selezionare la quantità da acquistare: ",program,articolo.id,articolo.nomeFarmaco,articolo.quantità,program);
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            faddArticolo(articolo,fsd);
            flock(fileno(fsd),LOCK_UN);
            fclose(fsd);
            return -1;
        }
        buffer[size]='\0';
        __uint32_t quantità=atoi(buffer);
        if((articolo.quantità>=quantità) && (quantità>=1))
        {
            if(indirizzo[0]!='\0')
            {
                FILE*fsd2;
                if(!(fsd2=fopen(tableSpedizioni,"a+")))
                {
                    fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
                    faddArticolo(articolo,fsd);
                    flock(fileno(fsd),LOCK_UN);
                    fclose(fsd);
                    return -1;
                }
                flock(fileno(fsd2),LOCK_EX);
                faddSpedizioni(indirizzo,articolo.nomeFarmaco,quantità,fsd2);
                flock(fileno(fsd2),LOCK_UN);
                fclose(fsd2);
            }
            dprintf(remote.sockfd,"%s: Farmaco comprato!\n",program);
            articolo.quantità-=quantità;
        }
        else
        {
            dprintf(remote.sockfd,"%s: Errore nell'acquisto del farmaco! Quantità non disponibile.\n",program);
        }
        faddArticolo(articolo,fsd);
    }
    else
    {
        dprintf(remote.sockfd,"%s: Errore: articolo non trovato!\n",program);
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
    Articolo articolo;
    char indirizzo[MAX_FORM_LENGTH];
    char buffer[N];
    ssize_t size;
    if((size=read(remote.sockfd,indirizzo,MAX_FORM_LENGTH))==-1)
    {
        fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    indirizzo[size]='\0';
    if(!strcmp(indirizzo,"\n"))
    {
        bzero(indirizzo,size);
    }
    dprintf(remote.sockfd,"\x1b[32m _____                                ______                   \n/  ___|                               |  ___|                  \n\\ `--.  ___ _ ____   _____ _ __ ______| |_ __ _ _ __ _ __ ___  \n `--. \\/ _ \\ '__\\ \\ / / _ \\ '__|______|  _/ _` | '__| '_ ` _ \\ \n/\\__/ /  __/ |   \\ V /  __/ |         | || (_| | |  | | | | | |\n\\____/ \\___|_|    \\_/ \\___|_|         \\_| \\__,_|_|  |_| |_| |_|\x1b[0m\n\n\n\n\n%s: Lista farmaci:\n",program);
    if((lista(remote))==-1)
    {
        close(remote.sockfd);
        return -1;
    }
    int e=0;
    while(true)
    {
        bzero(buffer,strlen(buffer));
        dprintf(remote.sockfd,"%s: Inserisci l'ID del farmaco da comprare (q per uscire): ",program);
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if(!strcmp(buffer,"q"))
        {
            dprintf(remote.sockfd,"%s: Arrivederci!",program);
            break;
        }
        else
        {
            if((acquisto(remote,atoi(buffer),indirizzo))==-1)
            {
                e=-1;
                break;
            }
        }
    }
    close(remote.sockfd);
    return e;
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
        close(remote.sockfd);
        return -1;
    }
    listen(locale.sockfd,backlog);
    if((creat(tableSpedizioni,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))==-1)
    {
        fprintf(stderr,"%s: creat: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    int e=0;
    pid_t pid;
    printf("%s: Attendo connessioni sulla porta %d…\n",program,ntohs(locale.addr.sin6_port));
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
            close(remote.sockfd);
            e=-1;
            break;
        }
        if(!pid)
        {
            close(locale.sockfd);
            if((connection(remote))==-1)
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