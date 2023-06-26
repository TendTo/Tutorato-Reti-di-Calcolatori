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
#include<signal.h>
#include<errno.h>
#define N 4096
#define MAX_NAME_LENGTH 255
#define MAX_USERNAME_LENGTH 20
#define MAX_MESSAGE_LENGTH 50
#define MAX_TRUST_VM 1024
typedef struct
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;
} SocketInfo;
typedef struct
{
	char username[MAX_USERNAME_LENGTH];
	char ip[INET_ADDRSTRLEN];
	char c[MAX_TRUST_VM];
	char messaggio[MAX_MESSAGE_LENGTH];
} Utente;
char program[MAX_NAME_LENGTH];

int reciver(SocketInfo remote)
{
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		memset(buffer,'\0',strlen(buffer));
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			e=-1;
			break;
		}
		buffer[size]=0;
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

int sender(SocketInfo remote, Utente utente)
{
	if((write(remote.sockfd,&utente,sizeof(Utente)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		return -1;
	}
	char buffer[N];
	while(true)
	{
		fgets(buffer,N,stdin);
		if(buffer[strlen(buffer)-1]=='\n')
		{
			buffer[strlen(buffer)-1]='\0';
		}
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
			return -1;
		}
		if(!strcmp(buffer,"q"))
		{
			break;
		}
	}
	return 0;
}

int sockaddrSettings(SocketInfo*sockaddr, char*address, int porta)
{
	sockaddr->addrlen=sizeof(struct sockaddr);
	if((sockaddr->sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	{
		fprintf(stderr,"%s: socket: %s\n",program,strerror(errno));
		return -1;
	}
	memset((char*)&sockaddr->addr,0,sockaddr->addrlen);
	sockaddr->addr.sin_family=AF_INET;
	sockaddr->addr.sin_addr.s_addr=(!address)?htonl(INADDR_ANY):inet_addr(address);
	sockaddr->addr.sin_port=htons(porta);
	return 0;
}

int client(char*username, char*c, char*messaggio, char*address, int porta)
{
	SocketInfo remote;
	if((sockaddrSettings(&remote,address,porta))==-1)
	{
		return -1;
	}
	Utente utente;
	strncpy(utente.username,username,MAX_USERNAME_LENGTH);
	strncpy(utente.c,c,MAX_TRUST_VM);
	strncpy(utente.messaggio,messaggio,MAX_MESSAGE_LENGTH);
	if((connect(remote.sockfd,(struct sockaddr*)&remote.addr,remote.addrlen))==-1)
	{
		fprintf(stderr,"%s: connect: %s\n",program,strerror(errno));
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
		if((reciver(remote))==-1)
		{
			kill(getppid(),SIGTERM);
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
	if((sender(remote,utente))==-1)
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
	strcpy(program,argv[0]);
	if(argc!=6)
	{
		printf("usage: %s <username> <utenti fidati> <messaggio> <indirizzo ip> <porta>\n",program);
	}
	else
	{
		if((client(argv[1],argv[2],argv[3],argv[4],atoi(argv[5])))==-1)
		{
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}