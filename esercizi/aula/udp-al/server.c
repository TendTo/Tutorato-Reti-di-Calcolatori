#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int sockfd, n;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char buffer[1000];
    
    if (argc != 3) {
        printf("Inserire Ip e porta\n");
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Errore nell'apertura del socket\n");
        return -1;
    }

    memset(&local_addr, 0, len);
    local_addr.sin_family = AF_INET;
    // local_addr.sin_addr.s_addr = inet_addr(argv[1]);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(atoi(argv[2]));

    if (bind(sockfd, (struct sockaddr*) &local_addr, len) < 0) {
        printf("Errore di binding\n");
        return -1;
    }

    while(1) {
        n = recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr*) &remote_addr, &len);
        buffer[n] = 0;

        printf("Messaggio: %s | da %s:%d\n", buffer, inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));

        sendto(sockfd, "ACK", 4, 0, (struct sockaddr*) &remote_addr, len);
    }

    close(sockfd);
    return 0;
}