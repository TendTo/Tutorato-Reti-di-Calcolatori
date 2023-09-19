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
#define backlog 10
#define INADDR_ANY6 "::"
#define MAX_FORM_LENGTH 32
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
volatile sig_atomic_t running=1;

/**
 * @brief Funzione che viene evocata quando viene catturato il signal di tipo interrupt, setta la variabile volatile running a 0.
 * @param signal in questo caso corrisponde al signal SIGINT.
*/
void signalHandler(int signal)
{
    running=0;
}

/**
 * @brief Gestisce l'ascolto con il server, tutti i messaggi inviati dal server vengono inviati allo stdout.
 * @param remote Struct contenente tutte informazioni della socket.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int receiver(SocketInfo remote)
{
    char buffer[N];
    ssize_t size;
    int e=0;
    while(running)
    {
        bzero(buffer,strlen(buffer));
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        if((write(1,buffer,size))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
    }
    close(remote.sockfd);
    return e;
}

/**
 * @brief Gestisce l'invio di messaggi verso il server, attende un messaggio proveniente dallo stdin e lo invia al server.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @param nomeAzienda Identifica l'azienda col Server.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int sender(SocketInfo remote, char*nomeAzienda)
{
    char buffer[N];
    ssize_t size;
    int e=0;
    while(true)
    {
        if(running)
        {
            fgets(buffer,N,stdin);
            buffer[strcspn(buffer,"\n")]='\0';
        }
        else
        {
            strcpy(buffer,"q");
        }
        if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        if(!strcmp(buffer,"a"))
        {
            Articolo articolo;
            strcpy(articolo.nomeAzienda,nomeAzienda);
            printf("Inserire il nome del vino: ");
            fgets(buffer,N,stdin);
            buffer[strcspn(buffer,"\n")]='\0';
            strcpy(articolo.nomeVino,buffer);
            printf("Inserire quantità: ");
            fgets(buffer,N,stdin);
            buffer[strcspn(buffer,"\n")]='\0';
            articolo.quantità=atoi(buffer);
            printf("Inserire costo: ");
            fgets(buffer,N,stdin);
            buffer[strcspn(buffer,"\n")]='\0';
            articolo.costo=atof(buffer);
            if((write(remote.sockfd,&articolo,sizeof(Articolo)))==-1)
            {
                fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
                e=-1;
                break;
            }
        }
        else if(!strcmp(buffer,"q"))
        {
            break;
        }
    }
    return e;
}

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
    if((inet_pton(AF_INET6,address,&sockaddr->addr.sin6_addr))!=1)
    {
        fprintf(stderr,"%s: inet_pton: %s\n",program,strerror(errno));
        close(sockaddr->sockfd);
        return -1;
    }
    //sockaddr->addr.sin6_addr=in6addr_any;
    sockaddr->addr.sin6_port=htons(porta);
    return 0;
}

/**
 * @brief Funzione che viene utilizzata come Client-Azienda nella comunicazione, richiede la connesione al server e se ha avuto successo il client si autentica tramite nomeAzienda, la comunicazione sfrutta il protocollo al livello trasporto: Trasmission Control Protocol
 * @param nomeAzienda nomeAzienda scelto dal Client-Azienda per autenticarsi con il server.
 * @param address Indirizzo IPv6 con il quale il client comunica col in server.
 * @param porta Porta che il server utilizza per reindirizzare tutti i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int client(char*nomeAzienda, char*address, in_port_t porta)
{
    SocketInfo remote;
    if((sockaddr6Settings(&remote,address,porta))==-1)
    {
        return -1;
    }
    char address1[INET6_ADDRSTRLEN];
    if(!inet_ntop(AF_INET6,&remote.addr.sin6_addr,address1,INET6_ADDRSTRLEN))
    {
        fprintf(stderr,"%s: inet_ntop: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    //printf("address1: %s\naddress: %s\n",address1,address);
    if((connect(remote.sockfd,(struct sockaddr*)&remote.addr,remote.addrlen))==-1)
    {
        fprintf(stderr,"%s: connect: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if((write(remote.sockfd,nomeAzienda,strlen(nomeAzienda)))==-1)
    {
        fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    pid_t pid;
    int e=0;
    if((pid=fork())==-1)
    {
        fprintf(stderr,"%s: fork: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if(!pid)
    {
        if((receiver(remote))==-1)
        {
            kill(getppid(),SIGTERM);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    if((sender(remote,nomeAzienda))==-1)
    {
        e=-1;
    }
    close(remote.sockfd);
    kill(pid,SIGTERM);
    return e;
}

int main(int argc, char**argv)
{
    extern int errno;
    strcpy(program,basename(argv[0]));
    signal(SIGINT,signalHandler);
    if(argc!=4)
    {
        printf("usage: %s <nome azienda> <address> <porta>\n",program);
    }
    else
    {
        if((client(argv[1],argv[2],atoi(argv[3])))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}