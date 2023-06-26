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
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#define N 4096
#define S_MUTEX 0
#define backlog 5
typedef struct
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t addrlen;
} SocketInfo;
typedef struct
{
	char username[backlog][20];
	char ip[backlog][INET_ADDRSTRLEN];
	char c[backlog][N];
	int sockfd[backlog];
	char messaggio[backlog][50];
	int id[backlog];
	bool isOnline[backlog];
} Utente;
char program[255];

int WAIT(int semid,int cond)
{
	struct sembuf operazioni[1]={{cond,-1,0}};
	return semop(semid,operazioni,1);
}

int SIGNAL(int semid, int cond)
{
	struct sembuf operazioni[1]={{cond,+1,0}};
	return semop(semid,operazioni,1);
}

//Session process

int inoltraMessaggio(int semid, SocketInfo remote, Utente*utente, int i)
{
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		memset(buffer,'\0',strlen(buffer));
		sprintf(buffer,"%s@%s: Scrivi l'username della persona di cui vuoi conoscere il segreto: ",program,utente->username[i]);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
			return -1;
		}
		memset(buffer,'\0',strlen(buffer));
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			return -1;
		}
		if(buffer[0]=='\n')
		{
			continue;
		}
		else
		{
			break;
		}
	}
	int j=0;
	WAIT(semid,S_MUTEX);
	while(j<=backlog-1)
	{
		if(i==j)
		{
			j-=-1;
			continue;
		}
		if(!strcmp(buffer,utente->username[j]))
		{
			for(char*token=strtok(utente->c[j]," "); token; token=strtok(NULL," "))
			{
				if(!strcmp(utente->username[i],token))
				{
					memset(buffer,'\0',strlen(buffer));
					sprintf(buffer,"%s: %s\n",utente->username[j],utente->messaggio[j]);
					SIGNAL(semid,S_MUTEX);
					if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
					{
						fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
						return -1;
					}
					return 0;
				}
			}	
			SIGNAL(semid,S_MUTEX);
			memset(buffer,'\0',strlen(buffer));
			sprintf(buffer,"%s: Errore: utente non trovato tra gli amici fidati di %s\n",program,utente->username[j]);
			if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
				return -1;
			}
			return 0;
		}
		j-=-1;
	}
	SIGNAL(semid,S_MUTEX);
	memset(buffer,'\0',strlen(buffer));
	sprintf(buffer,"%s: Errore: nessun username corrispondente\n",program);
	if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		return -1;
	}
	return 0;
}

int modificaMessaggio(int semid, SocketInfo remote, Utente*utente, int i)
{
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		memset(buffer,'\0',strlen(buffer));
		sprintf(buffer,"%s@%s: Scrivi il nuovo messaggio segreto: ",program,utente->username[i]);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
			return -1;
		}
		memset(buffer,'\0',strlen(buffer));
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			return -1;
		}
		if(buffer[0]=='\n')
		{
			continue;
		}
		else
		{
			break;
		}
	}
	WAIT(semid,S_MUTEX);
	strcpy(utente->messaggio[i],buffer);
	SIGNAL(semid,S_MUTEX);
	memset(buffer,'\0',strlen(buffer));
	sprintf(buffer,"%s: Messaggio segreto aggiornato\n",program);
	if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		return -1;
	}
	return 0;
}

int modificaUtentiFidati(int semid, SocketInfo remote, Utente*utente, int i)
{
	char buffer[N];
	ssize_t size;
	int e=0;
	while(true)
	{
		memset(buffer,'\0',strlen(buffer));
		sprintf(buffer,"%s@%s: Scrivi gli username che desideri siano fidati: ",program,utente->username[i]);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
			return -1;
		}
		memset(buffer,'\0',strlen(buffer));
		if((size=read(remote.sockfd,buffer,N))==-1)
		{
			fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
			return -1;
		}
		if(buffer[0]=='\n')
		{
			continue;
		}
		else
		{
			break;
		}
	}
	WAIT(semid,S_MUTEX);
	strcpy(utente->c[i],buffer);
	SIGNAL(semid,S_MUTEX);
	memset(buffer,'\0',strlen(buffer));
	sprintf(buffer,"%s: Lista utenti fidati aggiornata\n",program);
	if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
	{
		fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
		return -1;
	}
	return 0;
}

int session(int shmid, int semid, SocketInfo remote, int id, int nid)
{
	Utente*utente;
	char buffer[N];
	ssize_t size;
	char username[20];
	if((utente=(Utente*)shmat(shmid,NULL,0))==(Utente*)-1)
	{
		fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
		close(remote.sockfd);
		return -1;
	}
	if((size=read(remote.sockfd,buffer,N))==-1)
	{
		fprintf(stderr,"%s: read: %s\n",program,strerror(errno));
		close(remote.sockfd);
		return -1;
	}
	char*token=strtok(buffer,"\t");
	strcpy(username,token);
	int i;
	WAIT(semid,S_MUTEX);
	for(i=0; i<=nid-1; i-=-1)
	{
		if(!strcmp(utente->username[i],username))
		{
			if(utente->isOnline[i])
			{
				SIGNAL(semid,S_MUTEX);
				memset(buffer,'\0',strlen(buffer));
				sprintf(buffer,"%s: L'utente %s è già connesso!",program,username);
				if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
				{
					fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
					close(remote.sockfd);
					return -1;
				}
				return 0;
			}
			else
			{
				utente->isOnline[i]=true;
				strcpy(utente->ip[i],inet_ntoa(remote.addr.sin_addr));
				utente->sockfd[i]=remote.sockfd;
				token=strtok(NULL,"\t");
				strcpy(utente->c[i],token);
				token=strtok(NULL,"\t");
				strcpy(utente->messaggio[i],token);
				SIGNAL(semid,S_MUTEX);
				memset(buffer,'\0',strlen(buffer));
				sprintf(buffer,"%s: Accesso eseguito: utenti fidati e messaggio aggiornati\n",program);
				if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
				{
					fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
					close(remote.sockfd);
					return -1;
				}
				break;
			}
		}
	}
	if(i==nid)
	{
		if(nid<=backlog-1)
		{
			strcpy(utente->username[i],username);
			strcpy(utente->ip[i],inet_ntoa(remote.addr.sin_addr));
			token=strtok(NULL,"\t");
			strcpy(utente->c[i],token);
			utente->sockfd[i]=remote.sockfd;
			token=strtok(NULL,"\t");
			strcpy(utente->messaggio[i],token);
			utente->id[i]=i;
			utente->isOnline[i]=true;
			SIGNAL(semid,S_MUTEX);
			memset(buffer,'\0',strlen(buffer));
			/*sprintf(buffer,"%s: Registrazione eseguita\n",program);
			if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
				close(remote.sockfd);
				return -1;
			}*/
			dprintf(remote.sockfd,"%s: Registrazione eseguita\n",program);
		}
		else
		{
			SIGNAL(semid,S_MUTEX);
			memset(buffer,'\0',strlen(buffer));
			sprintf(buffer,"%s: Spazio esaurito nel server!\n",program);
			if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
				close(remote.sockfd);
				return -1;
			}
			return 0;
		}
	}
	int e=0;
	bool s=false;
	while(true)
	{
		memset(buffer,'\0',strlen(buffer));
		sprintf(buffer,"--------------------------------\n|  c - modifica utenti fidati  |\n|  m - modifica messaggio      |\n|  u - username e segreto      |\n|  q - esci                    |\n--------------------------------\n\n\n\n\n%s@%s$ Scelta: ",program,utente->username[i]);
		if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
		{
			fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
			e=-1;
			break;
		}
		while(true)
		{
			memset(buffer,'\0',strlen(buffer));
			if((size=read(remote.sockfd,buffer,N))==-1)
			{
				fprintf(stderr,"%s: read: %s",program,strerror(errno));
				e=-1;
				break;
			}
			if(!strcmp(buffer,"c"))
			{
				if((modificaUtentiFidati(semid,remote,utente,i))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"m"))
			{
				printf("Messaggio: %s\n",utente->messaggio[i]);
				if((modificaMessaggio(semid,remote,utente,i))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"u"))
			{
				if((inoltraMessaggio(semid,remote,utente,i))==-1)
				{
					e=-1;
				}
				break;
			}
			else if(!strcmp(buffer,"q"))
			{
				s=true;
				break;
			}
			else if(!strcmp(buffer,"\n"))
			{
				memset(buffer,'\0',strlen(buffer));
				sprintf(buffer,"%s@%s$ Scelta: ",program,utente->username[i]);
			}
			else
			{
				memset(buffer,'\0',strlen(buffer));
				sprintf(buffer,"%s: Comando sconosciuto\n%s@%s$ Scelta: ",program,program,utente->username[i]);
			}
			if((write(remote.sockfd,buffer,strlen(buffer)))==-1)
			{
				fprintf(stderr,"%s: write: %s\n",program,strerror(errno));
				e=-1;
				break;
			}
		}
		if((e==-1) || (s))
		{
			break;
		}
	}
	WAIT(semid,S_MUTEX);
	utente->isOnline[i]=false;
	SIGNAL(semid,S_MUTEX);
	dprintf(remote.sockfd,"%s: Logout eseguito\n",program);
	close(remote.sockfd);
	return e;
}

//Parent process

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

int setAllOffline(int shmid)
{
	Utente*utente;
	if((utente=(Utente*)shmat(shmid,NULL,0))==(Utente*)-1)
	{
		fprintf(stderr,"%s: shmat: %s\n",program,strerror(errno));
		return -1;
	}
	for(int i=0; i<=backlog-1; i-=-1)
	{
		utente->isOnline[i]=false;
	}
	return 0;
}

int createShmAndSem(int*shmid, int*semid)
{
	key_t key=IPC_PRIVATE;
	mode_t mode=S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	if((*shmid=shmget(key,sizeof(Utente),IPC_CREAT | IPC_EXCL | mode))==-1)
	{
		fprintf(stderr,"%s: shmget: %s\n",program,strerror(errno));
		return -1;
	}
	if((*semid=semget(key,1,IPC_CREAT | IPC_EXCL | mode))==-1)
	{
		fprintf(stderr,"%s: semget: %s\n",program,strerror(errno));
		return -1;
	}
	semctl(*semid,S_MUTEX,SETVAL,1);
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
	SocketInfo remote;
	remote.addrlen=sizeof(struct sockaddr);
	int shmid, semid, e=0, i=0, ni=0;
	pid_t pid;
	if((createShmAndSem(&shmid,&semid))==-1)
	{
		close(locale.sockfd);
		return -1;
	}
	if((setAllOffline(shmid))==-1)
	{
		close(locale.sockfd);
		return -1;
	}
	printf("%s: Ascolto sulla porta %d…\n",program,ntohs(locale.addr.sin_port));
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
			if((session(shmid,semid,remote,i,ni))==-1)
			{
				kill(getppid(),SIGTERM);
				exit(EXIT_FAILURE);
			}
			printf("killo il processo\n");
			exit(EXIT_SUCCESS);
		}
		if(ni<=backlog-1)
		{
			i-=-1;
			ni=i;
		}
		close(remote.sockfd);
	}
	close(locale.sockfd);
	shmctl(shmid,IPC_RMID,NULL);
	semctl(semid,0,IPC_RMID,0);
	return e;
}

int main(int argc, char**argv)
{
	extern int errno;
	strcpy(program,basename(argv[0]));
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
