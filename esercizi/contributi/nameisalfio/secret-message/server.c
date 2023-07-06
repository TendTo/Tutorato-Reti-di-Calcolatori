#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>

#define USERNAME_SIZE 20
#define SECRET_SIZE 50
#define LIST_SIZE 128

typedef enum {REGISTER = 'r', LOGIN = 'l', UPDATE = 'u', GET_SECRET = 'g', EXIT = 'e'} Operation;

typedef struct 
{
    Operation op;
    char usr[USERNAME_SIZE];
    char usr_secret[USERNAME_SIZE];
    char secret[SECRET_SIZE];
    char list[LIST_SIZE];
    char ip[INET6_ADDRSTRLEN];
}Message;

void handle(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

bool Login(Message* msg, FILE* fp)
{
    rewind(fp);
    char line[BUFSIZ];
    while(fgets(line, BUFSIZ, fp))
    {
        char* usr = strtok(line, " ");
        char* ip = strtok(NULL, " ");
        if (!strcmp(usr, msg->usr) && !strcmp(ip, msg->ip))
            return true;
    }
    return false;
}

bool Register(Message* msg, FILE* fp)
{
    if (Login(msg, fp))
        return false;

    fseek(fp, 0, SEEK_END);
    fprintf(fp, "%s %s %s %s\n", msg->usr, msg->ip, msg->secret, msg->list);
    fflush(fp);
    return true;
}

bool Update(Message* msg, FILE* fp)
{
    if (!Login(msg, fp))
        return false;
    
    FILE* fp_tmp;
    if(!(fp_tmp = fopen("Database_tmp.txt", "a+"))) handle("Error fopen\n");

    rewind(fp);
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, fp))
    {
        char* dup = strdup(line);
        char* usr = strtok(line, " ");
        if(strcmp(usr, msg->usr))
            fprintf(fp_tmp, "%s", dup);
        free(dup);
    }
    fprintf(fp_tmp, "%s %s %s %s\n", msg->usr, msg->ip, msg->secret, msg->list);
    fflush(fp_tmp);

    rename("Database_tmp.txt", "Database.txt");
    return true; 
}

char* GetSecret(Message* msg, FILE* fp)
{
    if(!Login(msg, fp)) 
        return "login failed";
    
    rewind(fp);
    char line[BUFSIZ];
    while(fgets(line, BUFSIZ, fp))
    {
        char* usr_secret = strtok(line, " ");
        if(!strcmp(msg->usr_secret, usr_secret))    
        {
            strtok(NULL, " ");
            char* secret = malloc(BUFSIZ);
            secret = strtok(NULL, " ");

            char* msg_list = strtok(NULL, "\n");
            char* usr = strtok(msg_list, " ");
            while (usr)
            {
                if (!strcmp(msg->usr, usr)) 
                    return secret;
                usr = strtok(NULL, " ");
            }
        }
    }
    return "Permission denied";
}

char* handle_request(Message* msg)
{
    FILE* fp;
    if(!(fp = fopen("Database.txt", "r+"))) handle("Error fopen\n");

    if(msg->op == 'r') // Register
    {
        if(Register(msg, fp))
            return "Register succesful";
        else
            return "Register failed";
    }
    if(msg->op == 'l') // Login
    {
        if(Login(msg, fp))
            return "Login succesful";
        else
            return "Login failed";
    }
    if(msg->op == 'u') // Update List or Update Secret
    {
        if(Update(msg, fp))
            return "Update succesful";
        else
            return "Update failed";
    }
    if(msg->op == 'g') // Get Secret
        return GetSecret(msg, fp);
}

int main(int argc, char* argv[])
{
    int sockfd, n;
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    char buffer[BUFSIZ];
    char ipv6[INET6_ADDRSTRLEN];
    Message msg;

    if(argc != 2) handle("Error argc\n");

    memset(&msg, 0, sizeof(Message));
    memset(&server_addr, 0, len);
    memset(&client_addr, 0, len);

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(atoi(argv[1]));

    if((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) handle("Error socket\n");
    if((bind(sockfd, (struct sockaddr*)&server_addr, len)) < 0) handle("Error bind\n");
    
    printf("[+]Server listening...\n\n");
    while(true)
    {
        if((n = recvfrom(sockfd, &msg, sizeof(Message), 0, (struct sockaddr*)&client_addr, &len)) < 0) handle("Error recvfrom\n");
        inet_ntop(AF_INET6, &client_addr.sin6_addr, msg.ip, len);
        strcpy(buffer, handle_request(&msg));
        printf("Outcome: %s\n", buffer);
        if((sendto(sockfd, buffer, BUFSIZ, 0, (struct sockaddr*)&client_addr, len)) < 0) handle("Error sendto\n");
    }
    return 0;
}