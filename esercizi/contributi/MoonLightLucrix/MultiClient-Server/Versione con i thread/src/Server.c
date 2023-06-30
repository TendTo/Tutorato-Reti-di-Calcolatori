/**
 * @author MoonLightLucrix
 * @link https://github.com/MoonLightLucrix
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<pthread.h>
#include<unistd.h>
#include<errno.h>
#define N 4096
#define true 1
#define false 0
#define backlog 5
typedef char bool;
char program[255];

typedef struct
{
	int sockfd;
	struct sockaddr_in address;
	socklen_t length;
} SocketInfo;

typedef struct
{
	int id;
	char username[20];
	int sockfd;
	char address[INET_ADDRSTRLEN];
	int porta;
	bool isOnline;
} Utente;

Utente utente[backlog];
int numeroUtenti;

/**
 * @brief Gestisce la sessione con il client.
 * @param data Spazio di memoria passato dal thread main contenente l'ID del client.
*/
void*connection(void*data)
{
	int idUtente=*((int*)data);
	char buffer[N];
	char buffer2[N];
	ssize_t size;
	while(true)
	{
		memset(buffer,0,N);
		if((size=read(utente[idUtente].sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("read");
			close(utente[idUtente].sockfd);
			pthread_exit(NULL);
		}
		if(!strcmp(buffer,"quit"))
		{
			break;
		}
		else if(!strcmp(buffer,"who"))
		{
			memset(buffer,0,N);
			sprintf(buffer,"%s: Elenco utenti:\n",program);
			if((write(utente[idUtente].sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: ",program);
				perror("write");
				close(utente[idUtente].sockfd);
				pthread_exit(NULL);
			}
			for(int i=0; i<=numeroUtenti-1; i-=-1)
			{
				if(utente[i].isOnline)
				{
					memset(buffer,0,N);
					sprintf(buffer,"%s\n",utente[i].username);
					if((write(utente[idUtente].sockfd,buffer,strlen(buffer)))==-1)
					{
						fprintf(stderr,"%s: ",program);
						perror("write");
						close(utente[idUtente].sockfd);
						pthread_exit(NULL);
					}
				}
			}
		}
		else
		{
			memset(buffer2,0,N);
			sprintf(buffer2,"%s: %s",utente[idUtente].username,buffer);
			for(int i=0; i<=numeroUtenti-1; i-=-1)
			{
				if(i!=idUtente)
				{
					if((write(utente[i].sockfd,buffer2,strlen(buffer)))==-1)
					{
						fprintf(stderr,"%s: ",program);
						perror("write");
						pthread_exit(NULL);
					}
				}
			}
		}
	}
	utente[idUtente].isOnline=false;
	close(utente[idUtente].sockfd);
	pthread_exit(NULL);
}

/**
 * @brief Funzione che genera ID per gli utenti che eseguono la registrazione o restituisce il vecchio ID agli utenti che vi accedono.
 * @param remote Struct contenente tutte le informazioni della socket.
 * @return Genera un nuovo ID per gli utenti che si registrano per la prima volta, se l'utente era già registrato la funzione restituisce il vecchio ID, se un utente con un username uguale ad un altro utente che ha già effettuato l'accesso la funzione restituisce il valore di backlog, restituisce il numero totale di utenti che coincide con il numero di backlog se si è raggiunta la capienza massima di registrazioni o restituisce -1 in caso di errore.
*/
int login(SocketInfo remote)
{
	char buffer[N];
	ssize_t size;
	memset(buffer,0,N);
	if((size=read(remote.sockfd,buffer,N))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("read");
		close(remote.sockfd);
		return -1;
	}
	printf("%s\n",buffer);
	for(int i=0; i<=numeroUtenti-1; i-=-1)
	{
		if(!strcmp(buffer,utente[i].username))
		{
			if(utente[i].isOnline)
			{
				return backlog;
			}
			utente[i].sockfd=remote.sockfd;
			strcpy(utente[i].address,inet_ntoa(remote.address.sin_addr));
			utente[i].porta=ntohs(remote.address.sin_port);
			utente[i].isOnline=true;
			memset(buffer,0,size);
			sprintf(buffer,"%s: Accesso effettuato\n",program);
			if((write(utente[i].sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: ",program);
				perror("write");
				close(remote.sockfd);
				return -1;
			}
			return i;
		}
	}
	if(numeroUtenti<=backlog-1)
	{
		for(int i=0; i<=numeroUtenti-1; i-=-1)
		{
			if(!strcmp(buffer,utente[i].username))
			{
				memset(buffer,0,size);
				sprintf(buffer,"%s: Questo username è già stato utilizzato",program);
				if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
				{
					fprintf(stderr,"%s: ",program);
					perror("write:");
					close(remote.sockfd);
					return -1;
				}
				close(remote.sockfd);
				return backlog;
			}
		}
		utente[numeroUtenti].id=numeroUtenti;
		strcpy(utente[numeroUtenti].username,buffer);
		utente[numeroUtenti].sockfd=remote.sockfd;
		strcpy(utente[numeroUtenti].address,inet_ntoa(remote.address.sin_addr));
		utente[numeroUtenti].porta=ntohs(remote.address.sin_port);
		utente[numeroUtenti].isOnline=true;
		numeroUtenti-=-1;
		memset(buffer,0,size);
		sprintf(buffer,"%s: Iscrizione effettuata\n",program);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("write");
			close(remote.sockfd);
			return -1;
		}
		return numeroUtenti-1;
	}
	else
	{
		memset(buffer,0,size);
		sprintf(buffer,"%s: Server saturo, riprova più tardi",program);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("write:");
			close(remote.sockfd);
			return -1;
		}
		close(remote.sockfd);
	}
	return numeroUtenti;
}

/**
 * @brief Funzione che si occupa della parte server della comunicazione.
 * @param porta Porta di comunicazione nella quale il server si mette in ascolto.
 * @return In caso di errore la funzione restituisce -1 o restituisce 0 altrimenti.
*/
int server(int porta)
{
	SocketInfo locale;
	if((locale.sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("socket");
		return -1;
	}
	locale.length=sizeof(locale.address);
	memset((char*)&locale.address,0,locale.length);
	locale.address.sin_family=AF_INET;
	locale.address.sin_addr.s_addr=inet_addr("0.0.0.0");
	locale.address.sin_port=htons(porta);
	if((bind(locale.sockfd,(struct sockaddr*)&locale.address,locale.length))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("bind");
		close(locale.sockfd);
		return -1;
	}
	listen(locale.sockfd,backlog);
	SocketInfo remote;
	remote.length=sizeof(remote.address);
	pthread_t th[backlog];
	numeroUtenti=0;
	int i=0;
	while(true)
	{
		printf("%s: Attendo connessioni sulla porta %d…\n",program,ntohs(locale.address.sin_port));
		if((remote.sockfd=accept(locale.sockfd,(struct sockaddr*)&remote.address,&remote.length))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("accept");
			close(locale.sockfd);
			return -1;
		}
		printf("%s: Tentivo di connessione da parte di %s\n",program,inet_ntoa(remote.address.sin_addr));
		i=login(remote);
		if((i<=backlog-1) && (i>=0))
		{
			pthread_create(&(th[i]),NULL,&connection,(void*)&i);
		}
	}
	return 0;
}

int main(int argc,char**argv)
{
	extern int errno;
	strcpy(program,argv[0]);
	if(argc!=2)
	{
		printf("Usage: %s <porta>\n",program);
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
