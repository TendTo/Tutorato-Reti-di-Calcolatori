/*
    Operazioni possibili:

    1) Chiedere l’elenco delle risorse disponibili (quindi con quantità > 0);
    2) Chiedere di prenotare una data risorsa (il server verifica se è ancora disponibile);
    3) Chiedere l’elenco delle risorse già prenotate;
    4) Liberare una risorsa prenotata in precedenza(il server verifica che era già riservata);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define NAME_SIZE 20
#define PORT 8000

#define OPERATION_1   "1) Chiedere l’elenco delle risorse disponibili (quindi con quantità > 0);\n"
#define OPERATION_2   "2) Chiedere di prenotare una data risorsa (il server verifica se è ancora disponibile);\n"
#define OPERATION_3   "3) Chiedere l’elenco delle risorse già prenotate;\n"
#define OPERATION_4   "4) Liberare una risorsa prenotata in precedenza(il server verifica che era già riservata);\n"
#define EXIT          "   [-]EXIT\n"

void handle_error(char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef struct
{
    char operation[BUFFER_SIZE];
    int ID;
    char resource[NAME_SIZE];
} Request;

void printRequest(Request* request)
{
    printf("\nRequest\n-----------\n");
    printf("Operation: %s\n", request->operation);
    printf("ID: %d\n", request->ID);
    printf("Resource: %s\n", request->resource);
}

int main(int argc, char* argv[])
{
    int sockfd, remote_sockfd, n;
    struct sockaddr_in server_addr, remote_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];

    if(argc != 3)
        handle_error("Error argc\n");

    memset(&server_addr, 0, len);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(argv[2]));

    memset(&remote_addr, 0, len);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = INADDR_ANY;
    remote_addr.sin_port = htons(PORT); 

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        handle_error("Error socket\n");

    if((remote_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        handle_error("Error socket\n");

    printf("Operazioni consentite:\n\n%s%s%s%s%s\n", OPERATION_1, OPERATION_2, OPERATION_3, OPERATION_4, EXIT);
    if(fork())
    {
        while(true)
        {
            printf("Enter a request: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;  

            if(strcasecmp(buffer, "EXIT") == 0) 
            {
                printf("\nGoodbye!\n");
                break;
            }
            else if(strcmp(buffer, "1") == 0 || strcmp(buffer, "2") == 0 || 
                    strcmp(buffer, "3") == 0 || strcmp(buffer, "4") == 0)
            {
                Request request;
                char str[NAME_SIZE];

                strcpy(request.operation, buffer);

                printf("\n-> Enter ID: ");   //Richiedo l'ID
                fgets(str, NAME_SIZE, stdin);
                str[strcspn(str, "\n")] = 0;
                request.ID = atoi(str);

                printf("-> Enter a resource: ");    //Richiedo la risorsa
                fgets(str, NAME_SIZE, stdin);
                str[strcspn(str, "\n")] = 0;
                strcpy(request.resource, str);

                if((sendto(sockfd, &request, sizeof(Request), 0, (struct sockaddr*) &server_addr, len)) < 0)
                    handle_error("Error sendto\n");

                if((n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, &len)) < 0)
                    handle_error("Error recvfrom\n");
                buffer[n] = 0;
                printf("\nOutcome: %s\n\n", buffer);
            }
            else
                printf("Invalid request\n\n");        

        }
        return 0;
    }
    else
    {
        if((bind(remote_sockfd, (struct sockaddr *)&remote_addr, len)) < 0)
            handle_error("Error bind\n");

        if((n = recvfrom(remote_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &remote_addr, &len)) < 0)
            handle_error("Error recvfrom\n");
        buffer[n] = 0;
        printf("\n\n[+]Outcome : %s\n", buffer);

        close(remote_sockfd);
    }

    return 0;
}