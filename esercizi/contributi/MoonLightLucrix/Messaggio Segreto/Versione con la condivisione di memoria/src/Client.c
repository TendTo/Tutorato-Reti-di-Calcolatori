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
typedef struct
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;
}SocketInfo;
char program[255];

int reciver(SocketInfo remote)
{
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			e=-1;
			break;
		}
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

int sender(SocketInfo remote)
{
	char buffer[N];
	while(true)
	{
		fgets(buffer,N,stdin);
		if((strlen(buffer)>=2) && (buffer[strlen(buffer)-1]=='\n'))
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
			memset(buffer,'\0',strlen(buffer));
			if((read(remote.sockfd,buffer,N))==-1)
			{
				fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
				return -1;
			}
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
	if((connect(remote.sockfd,(struct sockaddr*)&remote.addr,remote.addrlen))==-1)
	{
		fprintf(stderr,"%s: connect: %s\n",program,strerror(errno));
		close(remote.sockfd);
		return -1;
	}
	char buffer[N];
	memset(buffer,'\0',strlen(buffer));
	sprintf(buffer,"%s\t%s\t%s",username,c,messaggio);
	if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		close(remote.sockfd);
		return -1;
	}
	pid_t pid;
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
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
	int e=0;
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
	if(argc!=6)
	{
		printf("usage: %s <username> <c> <messaggio> <ip address> <porta>\n",program);
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
