#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sockfd, n;
    extern int errno;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    char mesg[1000];

    if (argc < 2)
    {
        printf("Use: server listening_PORT\n");
        return 0;
    }

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error in socket opening ... exit!");
        return -1;
    }

    memset(&local_addr, 0, len);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&local_addr, len) < 0)
    {
        perror("\nBinding error");
        return -1;
    }
    for (;;)
    {
        n = recvfrom(sockfd, mesg, 999, 0, (struct sockaddr *)&remote_addr, &len);
        mesg[n] = 0;
        printf("From IP:%s Port:%d msg:%s \n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), mesg);
        sendto(sockfd, mesg, n, 0, (struct sockaddr *)&remote_addr, len);
        printf("Mio indirizzo IP: %s\n", inet_ntoa(local_addr.sin_addr));
    }
}
