#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#include<libgen.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define N 4096
#define backlog 5
#define MAX_NAME_LENGTH 255
#define MAX_USERNAME_LENGTH 20
#define MAX_MESSAGE_LENGTH 50
#define MAX_TRUST_VM 1024
#define database "iplist.txt"
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
typedef enum {REPLACE_MESSAGE='m', REPLACE_TRUST_USERS='c', SEND_MESSAGE='u', QUIT='q', ACCESS} Operazione;
char program[MAX_NAME_LENGTH];

//Session process

void replace(FILE*fdd, char*line)
{
	rewind(fdd);
	FILE*tmp=tmpfile();
	char buffer[N];
	size_t size;
	while(fgets(buffer,N,fdd))
	{
		if(strcmp(buffer,line))
		{
			fputs(buffer,tmp);
		}
	}
	rewind(tmp);
	rewind(fdd);
	memset(buffer,'\0',strlen(buffer));
	do{
		size=fread(buffer,1,N,tmp);
		fwrite(buffer,1,size,fdd);
	}while(N==size);
	fclose(tmp);
	truncate(database,ftell(fdd));
}

char*find(FILE*fsd, char*username)
{
	char buffer[N];
	char*line=NULL;
	char ausername[MAX_USERNAME_LENGTH];
	while((line=fgets(buffer,N,fsd)))
	{
		sscanf(buffer,"%[^\t]",ausername);
		if(!strcmp(username,ausername))
		{
			break;
		}
	}
	return line;
}

int login(SocketInfo remote, FILE*fsd, Utente utente)
{
	char buffer[N];
	int sd=fileno(fsd);
	flock(sd,LOCK_EX);
	char*lineExists=find(fsd,utente.username);
	if(lineExists)
	{
		char line[strlen(lineExists)];
		strcpy(line,lineExists);
		char isOnline;
		sscanf(line,"%*[^\t]\t%*[^\t]\t%*[^\t]\t%*[^\t]\t%c",&isOnline);
		if(isOnline=='x')
		{
			dprintf(remote.sockfd,"%s: L'utente è già connesso, premi q per uscire\n",program);
			flock(sd,LOCK_UN);
			return -1;
		}
		replace(fsd,line);
	}
	fprintf(fsd,"%s\t%s\t%s\t%s\t%c\n",utente.username,utente.ip,utente.c,utente.messaggio,'x');
	fflush(fsd);
	flock(sd,LOCK_UN);
	dprintf(remote.sockfd,"%s: %s\n",program,(lineExists)?"Accesso eseguito: indirizzo ip, utenti fidati e messaggio aggiornati\n":"Iscrizione effettuata\n");
	/*memset(buffer,'\0',strlen(buffer));
	sprintf(buffer,"%s; %s\n",program,(lineExists)?"Accesso eseguito: indirizzo ip, utenti fidati e messaggio aggiornati\n":"Iscrizione effettuata\n");
	if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		return -1;
	}*/
	return 0;
}

FILE*getOrDropUtente(SocketInfo remote, Utente*utente, Operazione operazione)
{
	FILE*fsd;
	char*line;
	char buffer[N];
	ssize_t size;
	if(operazione==SEND_MESSAGE)
	{
		if(!(fsd=fopen(database,"r")))
		{
			fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
			return NULL;
		}
		dprintf(remote.sockfd,"%s: inserisci l'utente di cui vuoi scoprire il segreto: ",program);
	}
	else
	{
		if(!(fsd=fopen(database,"r+")))
		{
			fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
			return NULL;
		}
		if(operazione==REPLACE_MESSAGE)
		{
			dprintf(remote.sockfd,"%s: inserisci il nuovo messaggio: ",program);
		}
		else if(operazione==REPLACE_TRUST_USERS)
		{
			dprintf(remote.sockfd,"%s: inserisci i nuovi utenti fidati: ",program);
		}
	}
	if(operazione!=QUIT)
	{
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			fclose(fsd);
			return NULL;
		}
		buffer[size]=0;
		if(operazione==ACCESS)
		{
			memset(utente,0,sizeof(Utente));
			memcpy(utente,buffer,sizeof(Utente));
			strcpy(utente->ip,inet_ntoa(remote.addr.sin_addr));
		}
		else if(operazione==REPLACE_TRUST_USERS)
		{
			memset(utente->c,'\0',strlen(utente->c));
			strcpy(utente->c,buffer);
		}
		else if(operazione==REPLACE_MESSAGE)
		{
			memset(utente->messaggio,'\0',strlen(utente->messaggio));
			strcpy(utente->messaggio,buffer);
		}
		else if(operazione==SEND_MESSAGE)
		{
			memset(utente,0,sizeof(Utente));
			strcpy(utente->username,buffer);
		}
	}
	return fsd;
}

int leggiEModificaLinea(SocketInfo remote, Utente*utente, char*username, Operazione operazione)
{
	FILE*fsd;
	if(!(fsd=getOrDropUtente(remote,utente,operazione)))
	{
		return -1;
	}
	int e=0, sd;
	sd=fileno(fsd);
	flock(sd,LOCK_EX);
	char*lineExists=find(fsd,utente->username);
	if(lineExists)
	{
		flock(sd,LOCK_UN);
		char line[strlen(lineExists)];
		strcpy(line,lineExists);
		if(operazione==SEND_MESSAGE)
		{
			sscanf(line,"%*[^\t]\t%[^\t]\t%[^\t]\t%[^\t]",utente->ip,utente->c,utente->messaggio);
			if(strstr(utente->c,username))
			{
				dprintf(remote.sockfd,"%s: %s\n",utente->username,utente->messaggio);
			}
			else
			{
				dprintf(remote.sockfd,"%s: Errore: %s non è presente nella lista di utenti fidati di %s\n",program,username,utente->username);
			}
		}
		else
		{
			replace(fsd,line);
			fprintf(fsd,"%s\t%s\t%s\t%s\t%c\n",utente->username,utente->ip,utente->c,utente->messaggio,(operazione==QUIT)?'o':'x');
			fflush(fsd);
			flock(sd,LOCK_UN);
		}
	}
	else
	{
		dprintf(remote.sockfd,"%s: Errore: l'utente %s non esiste\n",program,utente->username);
		e=(operazione!=SEND_MESSAGE)?-1:0;
	}
	fclose(fsd);
	return e;
}

int session(SocketInfo remote)
{
	Utente utente;
	FILE*fsd;
	if(!(fsd=getOrDropUtente(remote,&utente,ACCESS)))
	{
		close(remote.sockfd);
		return -1;
	}
	if((login(remote,fsd,utente))==-1)
	{
		close(remote.sockfd);
		fclose(fsd);
		return 0;
	}
	fclose(fsd);
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		dprintf(remote.sockfd,"--------------------------------\n| u - username e messaggio     |\n| c - cambia utenti fidati     |\n| m - cambia messaggio segreto |\n| q - esci                     |\n--------------------------------\n\n\n\n\n\n\n%s@%s$ Scelta: ",program,utente.username);
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
			if(!strcmp(buffer,"c"))
			{
				if((leggiEModificaLinea(remote,&utente,NULL,REPLACE_TRUST_USERS))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"m"))
			{
				if((leggiEModificaLinea(remote,&utente,NULL,REPLACE_MESSAGE))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"u"))
			{
				Utente autente;
				if((leggiEModificaLinea(remote,&autente,utente.username,SEND_MESSAGE))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"q"))
			{
				if((leggiEModificaLinea(remote,&utente,NULL,QUIT))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"\n"))
			{
				dprintf(remote.sockfd,"%s@%s$ Scelta: ",program,utente.username);
				continue;
			}
			dprintf(remote.sockfd,"%s: comando non trovato\n%s@%s$ Scelta: ",program,program,utente.username);
		}
		if((e==-1) || (!strcmp(buffer,"q")))
		{
			break;
		}
	}
	close(remote.sockfd);
	return e;
}

//Listener process

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

int server(int porta)
{
	SocketInfo locale;
	if((sockaddrSettings(&locale,NULL,porta))==-1)
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
	pid_t pid;
	FILE*fsd;
	if(!(fsd=fopen(database,"w+")))
	{
		fprintf(stderr,"%s: fopen: %s\n",program,strerror(errno));
		close(locale.sockfd);
		return -1;
	}
	fclose(fsd);
	SocketInfo remote;
	remote.addrlen=sizeof(struct sockaddr);
	int e=0;
	printf("%s: Attendo connessioni sulla porta %d…\n",program,porta);
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
			e=-1;
			break;
		}
		if(!pid)
		{
			close(locale.sockfd);
			if((session(remote))==-1)
			{
				kill(getppid(),SIGTERM);
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
	if(argc!=2)
	{
		printf("usage: %s <porta>\n", program);
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