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
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#define N 4096
#define true 1
#define false 0
typedef char bool;
char program[255];
typedef struct
{
	int sockfd;
	struct sockaddr_in address;
	socklen_t length;
}SocketInfo;

/**
 * @brief Rutine che si occupa della ricezione da parte del server.
 * @param data Spazio di memoria passato dal thread main che continene la struct SocketInfo.
*/
void*reciver(void*data)
{
	SocketInfo remote=*((SocketInfo*)data);
	char buffer[N];
	ssize_t size;
	while(true)
	{
		memset(buffer,0,N);
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("read");
			close(remote.sockfd);
			pthread_exit(NULL);
		}
		if((write(1,buffer,size))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("write");
			close(remote.sockfd);
			pthread_exit(NULL);
		}
	}
	pthread_exit(NULL);
}

/**
 * @brief Rutine che si occupa di mandare messaggi al server.
 * @param data Spazio di memoria passato dal thread main che continene la struct SocketInfo.
*/
void*sender(void*data)
{
	SocketInfo remote=*((SocketInfo*)data);
	char buffer[N];
	while(true)
	{
		memset(buffer,0,N);
		fgets(buffer,N,stdin);
		if(buffer[strlen(buffer)-1]=='\n')
		{
			buffer[strlen(buffer)-1]='\0';
		}
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: ",program);
			perror("write");
			close(remote.sockfd);
			pthread_exit(NULL);
		}
		if(!strcmp(buffer,"quit"))
		{
			break;
		}
	}
	pthread_exit(NULL);
}

/**
 * @brief Funzione che si occupa della parte client della comunicazione.
 * @param address Indirizzo IPv4 col quale il client si mette in contatto con il Server
 * @param porta Porta col quale poi il server indirizza i messaggi inviati dal client.
 * @return In caso di errore la funzione restituisce -1 o restituisce 0 altrimenti.
*/
int client(char*username, char*address, int porta)
{
	SocketInfo remote;
	if((remote.sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("socket");
		return -1;
	}
	remote.length=sizeof(remote.address);
	memset((char*)&remote.address,0,remote.length);
	remote.address.sin_family=AF_INET;
	remote.address.sin_addr.s_addr=inet_addr(address);
	remote.address.sin_port=htons(porta);
	char buffer[N];
	ssize_t size;
	pthread_t th1, th2;
	if((connect(remote.sockfd,(struct sockaddr*)&remote.address,remote.length))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("connect");
		close(remote.sockfd);
		return -1;
	}
	if((write(remote.sockfd,username,strlen(username)))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("write");
		close(remote.sockfd);
		return -1;
	}
	if((size=read(remote.sockfd,buffer,N))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("read");
		close(remote.sockfd);
		return -1;
	}
	if((write(1,buffer,size))==-1)
	{
		fprintf(stderr,"%s: ",program);
		perror("write");
		close(remote.sockfd);
		return -1;
	}
	pthread_create(&th1,NULL,&reciver,(void*)&remote);
	pthread_create(&th2,NULL,&sender,(void*)&remote);
	pthread_join(th2,NULL);
	pthread_cancel(th1);
	close(remote.sockfd);
	return 0;
}

int main(int argc,char**argv)
{
	extern int errno;
	strcpy(program,argv[0]);
	if(argc!=4)
	{
		printf("Usage: %s <username> <address> <port>\n",program);
	}
	else
	{
		if((strlen(argv[2]))<=INET_ADDRSTRLEN)
		{
			if((client(argv[1],argv[2],atoi(argv[3])))==-1)
			{
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			printf("%s: indirizzo IP non corretto\n",program);
		}
	}
	exit(EXIT_SUCCESS);
}
