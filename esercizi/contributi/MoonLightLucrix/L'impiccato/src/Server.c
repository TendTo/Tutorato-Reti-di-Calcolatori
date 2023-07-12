#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>
#include<libgen.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<time.h>
#include<errno.h>
#define N 4096
#define MAX_FORM_LENGTH 32
#define MAX_CLIENT 2
#define MAX_SIZE_WORDLIST 5
#define max 3
typedef struct
{
    char nome[MAX_FORM_LENGTH];
    char address[INET_ADDRSTRLEN];
    __uint16_t porta;
} Client;
char program[FILENAME_MAX];

/**
 * @brief Setta tutti le varie impostazioni per la socket e la struct sockaddr_in.
 * @param sockfd File descriptor della socket.
 * @param addr Struct dove settare tutte le informazioni legate all'indirizzo ip, numero di porta e famiglia della socket.
 * @param addrlen Lunghezza della struct sockaddr_in o struct sockaddr (hanno lo stesso numero di byte).
 * @param addess Indirizzo IPv4 a cui mandare pacchetti, se viene passata come NULL verrà impostata a INADDR_ANY.
 * @param porta Porta sulla quale indirizzare i pacchetti entranti.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int sockaddrSettings(int*sockfd, struct sockaddr_in*addr, socklen_t*addrlen, char*address, __uint16_t porta)
{
    *addrlen=sizeof(struct sockaddr);
    if((*sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
    {
        fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
        return -1;
    }
    memset((char*)addr,0,*addrlen);
    addr->sin_family=AF_INET;
    addr->sin_addr.s_addr=(!address)?INADDR_ANY:inet_addr(address);
    addr->sin_port=htons(porta);
    return 0;
}

/**
 * @brief Setta un vettore bidimensionale di parole e ne sceglie una randomicamente.
 * @return Un puntatore ad una copia dinamica della parola scelta.
*/
char*getWord()
{
    srand(time(NULL));
    char*wordList[MAX_SIZE_WORDLIST]={"situazione","esplodere","giro","testimone","popolo"};
    return strdup(wordList[rand()%MAX_SIZE_WORDLIST]);
}

/**
 * @brief Verifica se il char o la stringa passata dal client è: nel caso di un char presente all'interno della parola da indovinare se no nel caso di una stringa confronta con la parola in chiaro se sono uguali, se viene indovinato l'ultimo char o la stringa passata dal client è uguale all stringa il giocatore che ha mandato il messaggio vince, vengono conteggiate delle vite, se il giocatore arriva all'ultima vita e sbaglia viene escluso dalla partita, se entrambi i giocatori perdono viene visualizzato a schermo Game Over ed il gioco termina.
 * @param sockfd File descriptor della socket.
 * @param addr Struct che contiene tutte le informazioni legate all'indirizzo ip, numero di porta e famiglia della socket.
 * @param addrlen Lunghezza della struct sockaddr_in o struct sockaddr (hanno lo stesso numero di byte).
 * @param buffer Stringa passata dal client, se la sua dimensione è 1 viene verificato se il character è prensete nella parola altrimenti confronta le due stringhe.
 * @param word Parola passata in chiaro per la verifica.
 * @param maskedWord Parola non in chiaro che in caso di successo viene o meno in progressione rivelata.
 * @param vita Vettore contenente il numero di vite del giocatore, se il contatore arriva a 0 il giocatore perde, se tutte le vite di tutti i giocatori arrivano a 0 il gioco termina e viene stampato a schermo sullo stdout "Game Over".
 * @param i Indice che tiene conto del giocatore corrente.
 * @param gameOver diventa true quando tutte le vite dei giocatori arrivano a 0.
 * @return In caso di errore verrà restituito -1, altrimenti 0.
*/
int impiccato(int sockfd, struct sockaddr_in*addr, socklen_t addrlen, char*buffer, char*word, char*maskedWord, int*vita, int i, bool*gameOver)
{
    if(strlen(buffer)==1)
    {
        bool found=false;
        for(int j=0; (j<=strlen(word)-1) && (!found); j-=-1)
        {
            if((!((tolower(buffer[0]))^word[j])) && (!(maskedWord[j]^'_')))
            {
                maskedWord[j]=word[j];
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"%s: Lettera indovinata!!\nParola: %s\n",program,maskedWord);
                if(!strchr(maskedWord,'_'))
                {
                    sprintf(buffer,"%s%s: \x1b[32mHai vinto!\x1b[0m\nParola: \x1b[4m%s\x1b[0m",buffer,program,word);
                }
                found=true;
            }
        }
        if(!found)
        {
            vita[i]--;
            if(!vita[i])
            {
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"%s: \x1b[31mHai perso.\x1b[0m\n",program);
                for(int j=0; j<=MAX_CLIENT-1; j-=-1)
                {
                    *gameOver|=(vita[j]);
                }
                *gameOver=(!(*gameOver));
            }
            else
            {
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"%s: Vite rimaste %d\n",program,vita[i]);
            }
        }
    }
    else
    {
        if(!strcasecmp(buffer,word))
        {
            strcpy(maskedWord,word);
            memset(buffer,'\0',strlen(buffer));
            sprintf(buffer,"%s: Parola indovinata!!\n",program);
            sprintf(buffer,"%s%s: \x1b[32mHai vinto!\x1b[0m\nParola: \x1b[4m%s\x1b[0m",buffer,program,word);
        }
        else
        {
            vita[i]--;
            if(!vita[i])
            {
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"%s: \x1b[31mHai perso.\x1b[0m\n",program);
                for(int j=0; j<=MAX_CLIENT-1; j-=-1)
                {
                    *gameOver|=(vita[j]);
                }
                *gameOver=(!(*gameOver));
            }
            else
            {
                memset(buffer,'\0',strlen(buffer));
                sprintf(buffer,"%s: Vite rimaste %d\n",program,vita[i]);
            }
        }
    }
    if((sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr*)addr+i,addrlen))==-1)
    {
        fprintf(stderr,"%s: sendto: %s\n",program,strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief Funzione che viene utilizzata come server nella comunicazione, setta tutte le impostazioni iniziali come la parola da scegliere o i vari settaggi per la connessione ed aspetta che tutti i client si connettino per far partire il gioco, il protocollo utilizzato è User Datagram Protocol.
 * @param porta Porta sulla quale vengono reindirizzati i pacchetti in entrata.
 * @return Restituisce -1 in caso di errore, 0 altrimenti.
*/
int server(__uint16_t porta)
{
    int sockfd;
    struct sockaddr_in lAddr, addr[MAX_CLIENT];
    socklen_t addrlen;
    if((sockaddrSettings(&sockfd,&lAddr,&addrlen,NULL,porta))==-1)
    {
        return -1;
    }
    if((bind(sockfd,(struct sockaddr*)&lAddr,addrlen))==-1)
    {
        fprintf(stderr,"%s: bind: %s\n",program,strerror(errno));
        close(sockfd);
        return -1;
    }
    char*wordt=getWord();
    char word[strlen(wordt)];
    strcpy(word,wordt);
    free(wordt);
    char maskedWord[strlen(word)];
    strcpy(maskedWord,word);
    memset(maskedWord,'_',strlen(maskedWord));
    //printf("Soluzione: \x1b[4m%s\x1b[0m\nLunghezza parola: %lu\nLungezza parola maschera: %lu\n",word,strlen(word),strlen(maskedWord));
    char buffer[N];
    ssize_t size;
    int e=0;
    Client client[MAX_CLIENT];
    int vita[MAX_CLIENT]={max,max};
    bool gameOver=false, gameStarted=false;
    printf("\x1b[32mAttendo connessioni sulla porta %d…\x1b[0m\n",ntohs(lAddr.sin_port));
    for(int i=0; true; i-=-1)
    {
        if(gameStarted)
        {
            i=i%MAX_CLIENT;
            printf("i: %d\n",i);
            if(!vita[i])
            {
                printf("ciao\n");
                continue;
            }
            memset(buffer,'\0',strlen(buffer));
            sprintf(buffer,"%s: È il turno del giocatore %d\nParole indovinate: %s\nInserisci una lettera o una parola: ",program,i+1,maskedWord);
            if((sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr*)addr+i,addrlen))==-1)
            {
                fprintf(stderr,"%s: sendto: %s\n",program,strerror(errno));
                e=-1;
                break;
            }
        }
        memset(buffer,'\0',strlen(buffer));
        if((size=recvfrom(sockfd,buffer,N,0,(struct sockaddr*)addr+i,&addrlen))==-1)
        {
            fprintf(stderr,"%s: recvfrom: %s\n",program,strerror(errno));
            e=-1;
            break;
        }
        buffer[size]='\0';
        printf("buffer: %s\n",buffer);
        if(!gameStarted)
        {
            if((i+2)<=MAX_CLIENT)
            {
                printf("Attendo \x1b[34mGiocatore %d\x1b[0m…\n",i+2);
            }
            strncpy(client[i].nome,buffer,MAX_FORM_LENGTH);
            client[i].nome[strlen(client[i].nome)]='\0';
            strcpy(client[i].address,inet_ntoa(addr[i].sin_addr));
            client[i].porta=ntohs(addr[i].sin_port);
            gameStarted=(i==MAX_CLIENT-1);
        }
        else
        {
            if((impiccato(sockfd,addr,addrlen,buffer,word,maskedWord,vita,i,&gameOver))==-1)
            {
                e=-1;
                break;
            }
        }
        if((!strcmp(word,maskedWord)) || (gameOver))
        {
            break;
        }
    }
    if(gameOver)
    {
        printf("\x1b[1;91mGAME OVER\x1b[0m\n");
    }
    close(sockfd);
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
}