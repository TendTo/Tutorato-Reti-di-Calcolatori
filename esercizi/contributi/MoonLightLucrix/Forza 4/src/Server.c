#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<libgen.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#define N 4096
#define MAX_FORM_LENGTH 32
#define MAX_CLIENT 2
#define righe 6
#define colonne 7
#define backlog 2
#define vuoto ' '
#define gettone 48
typedef struct
{
    char nome[MAX_FORM_LENGTH];
    in_addr_t address;
    __uint16_t porta;
} Client;
typedef struct
{
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen;
} SocketInfo;
char program[FILENAME_MAX];

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
 * @brief Inserisce nella griglia il gettone in base alla colonna scelta dal client.
 * @param griglia Griglia di gioco dove effettuare l'inserimento.
 * @param colonna colonna scelta dal client. 
 * @param i Client corrente. 
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int inserisci(char griglia[righe][colonne], int colonna, int i)
{
    for(int j=0; j<=righe-1; j-=-1)
    {
        if(griglia[j][colonna]==vuoto)
        {
            griglia[j][colonna]=(char)i+gettone;
            return j;
        }
    }
    return -1;
}

/**
 * @brief Effettua un controllo su tutta la linea verticale della colonna in cui il client ha effettuato l'inserimento se la colonna contiene 4 gettoni uguali di fila il gioco termina ed il clien i-esimo vince.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @param colonna Colonna dove effettuare il controllo
 * @param i Client corrente. 
 * @return TRUE se la colonna possiede 4 gettoni uguali (dello stesso client) di fila, FALSE altrimenti.
*/
bool controllaVerticale(char griglia[righe][colonne], int colonna, int i)
{
    int c=0;
    for(int j=0; j<=righe-1; j-=-1)
    {
        if(griglia[j][colonna]==(char)i+gettone)
        {
            c-=-1;
            if(c==4)
            {
                return true;
            }
        }
        else
        {
            c=0;
        }
    }
    return false;
}

/**
 * @brief Effettua un controllo su tutta la line orizzontale della riga in cui il client ha effettuato l'inserimento se la riga contiene 4 gettoni uguali di fila il gioco termina ed il clien i-esimo vince.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @param riga Riga dove effettuare il controllo
 * @param i Client corrente. 
 * @return TRUE se la riga possiede 4 gettoni uguali (dello stesso client) di fila, FALSE altrimenti.
*/
bool controllaOrizzontale(char griglia[righe][colonne], int riga, int i)
{
    int c=0;
    for(int j=0; j<=colonne-1; j-=-1)
    {
        if(griglia[riga][j]==(char)i+gettone)
        {
            c-=-1;
            if(c==4)
            {
                return true;
            }
        }
        else
        {
            c=0;
        }
    }
    return false;
}

/**
 * @brief Effettua un controllo su tutta la diagonale della colonna e riga in cui il client ha effettuato l'inserimento se la diagonale contiene 4 gettoni uguali di fila il gioco termina ed il clien i-esimo vince.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @param riga Riga dove è stato inserito il gettone
 * @param colonna Colonna dove è stato inserito il gettone
 * @param i Client corrente. 
 * @return TRUE se la diagonale possiede 4 gettoni uguali (dello stesso client) di fila, FALSE altrimenti.
*/
bool controllaDiagonale(char griglia[righe][colonne], int riga, int colonna, int i)
{
    int j, k, c=0;
    j=-(k=colonna-riga);
    for(j=(j<=-1)?0:j, k=(k<=-1)?0:k; (j<=righe-1) && (k<=colonne-1); j-=-1, k-=-1)
    {
        if(griglia[j][k]==(char)i+gettone)
        {
            c-=-1;
            if(!(c^4))
            {
                return true;
            }
        }
        else
        {
            c=0;
        }
    }
    return false;
}

/**
 * @brief Effettua un controllo su tutta la diagonale secondaria della colonna e riga in cui il client ha effettuato l'inserimento se la diagonale contiene 4 gettoni uguali di fila il gioco termina ed il clien i-esimo vince.
 * @authors Si ringraziano Antonino Fassari (MatrixNeoTwitch) e Ciccio Pennisi per l'aiuto nel ragionamento della funzione.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @param riga Riga dove è stato inserito il gettone
 * @param colonna Colonna dove è stato inserito il gettone
 * @param i Client corrente. 
 * @return TRUE se la diagonale secondaria possiede 4 gettoni uguali (dello stesso client) di fila, FALSE altrimenti.
*/
bool controllaSDiagonale(char griglia[righe][colonne], int riga, int colonna, int i)
{
    //printf("Inizio controllo\n");
    int j, k, c=0;
    //printf("riga: %d\ncolonna: %d\n",riga,colonna);
    j=-(k=((colonne-1)-colonna)-riga);
    //printf("j: %d\nk: %d\n",j,k);
    for(j=(j<=-1)?0:j, k=(k<=-1)?colonne-1:(colonne-1)-k; (j<=righe-1) && (k>=0); j-=-1, k--)
    {
        //printf("j: %d\nk: %d\n",j,k);
        //fflush(stdout);
        if(griglia[j][k]==(char)i+gettone)
        {
            c-=-1;
            if(!(c^4))
            {
                return true;
            }
        }
        else
        {
            c=0;
        }
    }
    return false;
}

/**
 * @brief Controlla se l'ultima riga non è vuota, se l'ultima riga non è vuota vuol dire che il gioco è terminato in pareggio perché tutte le caselle sono già state occupate.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @return TRUE se l'ultima riga è piena, FALSE altrimenti.
*/
bool isFull(char griglia[righe][colonne])
{
    bool count=true;
    for(int i=righe-1, j=0; j<=colonne-1; j-=-1)
    {
        count&=(griglia[i][j]!=vuoto);
    }
    return count;
}

/**
 * @brief Inizializza la stringa buffer con la stampa della griglia, l'equivalente del metodo toString.
 * @param griglia Griglia di gioco dove effettuare il controllo.
 * @param buffer Stringa dove stampare la griglia.
*/
void strGriglia(char griglia[righe][colonne], char*buffer)
{
    char*color[MAX_CLIENT]={"\x1b[34mo\x1b[0m","\x1b[31mo\x1b[0m"};
    for(int j=righe-1; j>=0; j--)
    {
        sprintf(buffer,"%s|",buffer);
        for(int k=0; k<=colonne-1; k-=-1)
        {
            if(griglia[j][k]==(char)gettone)
            {
                sprintf(buffer,"%s%s|",buffer,color[0]);
            }
            else if(griglia[j][k]==(char)gettone+1)
            {
                sprintf(buffer,"%s%s|",buffer,color[1]);
            }
            else
            {
                sprintf(buffer,"%s%c|",buffer,griglia[j][k]);
            }
        }
        sprintf(buffer,"%s\n",buffer);
    }
    for(int k=0; k<=colonne-1; k-=-1)
    {
        sprintf(buffer,"%s %d",buffer,k+1);
    }
    sprintf(buffer,"%s\n",buffer);
    //printf("%s\n",buffer);
}

/**
 * @brief Gestisce il gioco del Forza 4 chiedendo al Client i-esimo di inserire un valore compreso tra 1 e 7 (le colonne totali della griglia) e richiamando le varie funzioni di verifica dopo l'inserimento.
 * @param remote Struct contenete tutte le informazioni della socket di sessione con il client i-esimo.
 * @param griglia Griglia di gioco dove effettuare l'inserimento ed il controllo.
 * @param i Client corrente.
 * @param gameOver TRUE se la griglia è piena, quindi la partita finisce in pareggio, FALSE altrimenti.
 * @param thereISAWinner TRUE se è l'ultimo Client che ha effettuato l'inserimento ha vinto, FALSE altrimenti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int forza4(SocketInfo remote, char griglia[righe][colonne], int i, bool*gameOver, bool*thereIsAWinner)
{
    char buffer[N];
    ssize_t size;
    memset(buffer,'\0',strlen(buffer));
    strGriglia(griglia,buffer);
    sprintf(buffer,"%s%s: %sInserisci il valore della colonna dove vuoi inserire: %s",buffer,program,(!i)?"\x1b[34m":"\x1b[31m","\x1b[0m");
    if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
    {
        fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
        return -1;
    }
    int colonna, riga;
    while(true)
    {
        memset(buffer,'\0',strlen(buffer));
        if((size=read(remote.sockfd,buffer,N))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            return -1;
        }
        colonna=atoi(buffer)-1;
        if((-1>=colonna) || (7<=colonna))
        {
            dprintf(remote.sockfd,"%s: La colonna non esiste\n%s: %sInserisci il valore della colonna dove vuoi inserire: %s",program,program,(!i)?"\x1b[34m":"\x1b[31m","\x1b[0m");
        }
        else
        {
            if((riga=inserisci(griglia,colonna,i))==-1)
            {
                dprintf(remote.sockfd,"%s: La colonna non è disponibile\n%s: %sInserisci il valore della colonna dove vuoi inserire: %s",program,program,(!i)?"\x1b[34m":"\x1b[31m","\x1b[0m");
                continue;
            }
            break;
        }
    }
    memset(buffer,'\0',strlen(buffer));
    strGriglia(griglia,buffer);
    if((*thereIsAWinner=((controllaOrizzontale(griglia,riga,i)) || (controllaVerticale(griglia,colonna,i)) || (controllaDiagonale(griglia,riga,colonna,i)) || (controllaSDiagonale(griglia,riga,colonna,i)))))
    {
        sprintf(buffer,"%s\n%s: %sHai Vinto!%s\n",buffer,program,(!i)?"\x1b[1;34m":"\x1b[1;31m","\x1b[0m");
        if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            return -1;
        }
    }
    else if((*gameOver=(isFull(griglia))))
    {
        sprintf(buffer,"%s\n\x1b[1mPareggio!\x1b[0m\n",buffer);
        if((write(1,buffer,strlen(buffer)))==-1)
        {
            fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Funzione che viene utilizzata come server nella comunicazione, attende che tutti i gli accept dei client partecipanti al gioco, nel frattempo per ogni accept il server salva sulla struct "Client" ogni informazione del client: nome, ip address e porta, arrivati al numero massimo di client il gioco del Forza 4 comincia, il protocollo utilizzato è Trasmission Control Protocol.
 * @param porta Porta sulla quale vengono reindirizzati i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int server(__uint16_t porta)
{
    SocketInfo locale, remote[MAX_CLIENT];
    if((sockaddrSettings(&locale,NULL,porta))==-1)
    {
        return -1;
    }
    //setsockopt(locale.sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int)); Evito la rottura di palle di cambiare porta per ogni run del programma in fase di debugging.
    if((bind(locale.sockfd,(struct sockaddr*)&locale.addr,locale.addrlen))==-1)
    {
        fprintf(stderr,"%s: bind: %s\n",program,strerror(errno));
        close(locale.sockfd);
        return -1;
    }
    listen(locale.sockfd,backlog);
    for(int i=0; i<=backlog-1; i-=-1)
    {
        remote[i].addrlen=sizeof(struct sockaddr);
    }
    char griglia[righe][colonne];
    for(int i=0; i<=righe-1; i-=-1)
    {
        for(int j=0; j<=colonne-1; j-=-1)
        {
            griglia[i][j]=vuoto;
        }
    }
    Client client[MAX_CLIENT];
    char buffer[N];
    ssize_t size;
    int e=0;
    for(int i=0; i<=MAX_CLIENT-1; i-=-1)
    {
        printf("\x1b[32mAttendo il giocatore %d…\x1b[0m\n",i+1);
        if((remote[i].sockfd=accept(locale.sockfd,(struct sockaddr*)&remote[i].addr,&remote[i].addrlen))==-1)
        {
            fprintf(stderr,"%s: accept: %s\n",program,strerror(errno));
            close(locale.sockfd);
            return -1;
        }
        memset(buffer,'\0',strlen(buffer));
        if((size=read(remote[i].sockfd,buffer,MAX_FORM_LENGTH))==-1)
        {
            fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
            close(locale.sockfd);
            return -1;
        }
        buffer[size]='\0';
        strcpy(client[i].nome,buffer);
        client[i].address=remote[i].addr.sin_addr.s_addr;
        client[i].porta=ntohs(remote[i].addr.sin_port);
    }
    bool gameOver=false, thereIsAWinner=false; 
    for(int i=0; (!gameOver) && (!thereIsAWinner); i-=-1)
    {
        i=(i%MAX_CLIENT);
        if((forza4(remote[i],griglia,i,&gameOver,&thereIsAWinner))==-1)
        {
            e=-1;
            break;
        }
    }
    for(int i=0; i<=MAX_CLIENT-1; i-=-1)
    {
        shutdown(remote[i].sockfd,SHUT_RDWR);
        close(remote[i].sockfd);
    }
    shutdown(locale.sockfd,SHUT_RDWR);
    close(locale.sockfd);
    return e;
}

int main(int argc, char**argv)
{
    extern int errno;
    strcpy(program,basename(argv[0]));
    if(argc!=2)
    {
        printf("\x1b[31musage: %s <porta>\x1b[0m\n",program);
    }
    else
    {
        if((server(atoi(argv[1])))==-1)
        {
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);