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
#define IN6ADDR_ANY "::"
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
    running^=1;
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
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int sender(SocketInfo remote)
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
        if(!strcmp(buffer,"q"))
        {
            break;
        }
    }
    return e;
}

/**
 * @brief Funzione che viene utilizzata come client nella comunicazione, richiede la connesione al server, se si tratta di un Cliente Remoto la stringa indirizzo conterrà l'indirizzo da mandare al Server se no verrà inviato \\n ad indicare che si tratta di un Cliente Presente, la comunicazione sfrutta il protocollo al livello trasporto: Trasmission Control Protocol
 * @param address Indirizzo IPv6 con il quale il client comunica col in server.
 * @param indirizzo Indirizzo del Cliente Remoto, nel caso in cui il client sia un Cliente Remoto.
 * @param porta Porta che il server utilizza per reindirizzare tutti i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int client(char*indirizzo, char*address, in_port_t porta)
{
    SocketInfo remote;
    if((sockaddr6Settings(&remote,address,porta))==-1)
    {
        return -1;
    }
    if((connect(remote.sockfd,(struct sockaddr*)&remote.addr,remote.addrlen))==-1)
    {
        fprintf(stderr,"%s: connect: %s\n",program,strerror(errno));
        close(remote.sockfd);
        return -1;
    }
    if(!indirizzo)
    {
        dprintf(remote.sockfd,"\n");
    }
    else
    {
        if((write(remote.sockfd,indirizzo,strlen(indirizzo)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            close(remote.sockfd);
            return -1;
        }
    }
    pid_t pid;
    int e=0;
    if((pid=fork())==-1)
    {
        fprintf(stderr,"%s: accept: %s\n",program,strerror(errno));
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
    if((sender(remote))==-1)
    {
        e=-1;
    }
    kill(pid,SIGTERM);
    close(remote.sockfd);
    return e;
}

int main(int argc, char**argv)
{
    extern int errno;
    strcpy(program,basename(argv[0]));
    signal(SIGINT,signalHandler);
    if(argc==3)
    {
        if((client(NULL,argv[1],atoi(argv[2])))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(argc==4)
    {
        if((client(argv[1],argv[2],atoi(argv[3])))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("usage: %s <address> <porta>\n       %s <indirizzo> <address> <porta>\n",program,program);
    }
    exit(EXIT_SUCCESS);
}