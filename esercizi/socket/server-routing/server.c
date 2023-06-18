/**
 * @file server.c
 * @author Tend (casablancaernesto@gmail.com)
 * @brief Servizio che utilizza un server di routing per inviare messaggi ad un altro client
 * All'avvio del programma, verrà indicato al server su quale porta dovrà mettersi in ascolto,
 * la porta che verrà usata dai client per ricevere i messaggi ed un numero arbitrario di IP,
 * che corrispondono agli altri server di routing noti a questo server (la porta è assunta essere la stessa per tutti i server di routing).
 * @version 0.1
 * @date 2023-06-14
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

#define MAX_MSG_SIZE 2000

/**
 * @brief Controlla se l'indirizzo IP passato come parametro è nella stessa LAN del server
 * @param ip indirizzo IP da controllare
 * @return 1 se l'indirizzo IP è nella stessa LAN del server, 0 altrimenti
 */
int is_ip_in_lan(in_addr_t ip)
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        int mask = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
        int ip_server = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
        if ((ip & mask) == (ip_server & mask)) {
            freeifaddrs(ifap);
            return 1;
        }
    }
    freeifaddrs(ifap);
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr, other_server_addr;
    socklen_t server_len = sizeof(server_addr), client_len = sizeof(client_addr);

    // Il buffer deve essere abbastanza grande da contenere l'indirizzo IP del client destinatario, uno spazio e il messaggio che si vuole inviare
    // Esempio:
    // 10.0.0.1 Ciao come stai?
    // 192.168.1.2 Bene, tu?
    char buffer[INET_ADDRSTRLEN + MAX_MSG_SIZE + 1];

    if (argc < 3 || atoi(argv[1]) == 0 || atoi(argv[2]) == 0)
    {
        fprintf(stderr, "use: %s <porta d'ascolto server> <porta d'ascolto client> [ipServer1 ipServer2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    char **server_ips = argv + 3;
    int num_servers = argc - 3;

    // Azzera tutte le strutture dati. '\0' e 0 sono equivalenti
    memset(&server_addr, 0, server_len);
    memset(&client_addr, 0, client_len);
    memset(&other_server_addr, 0, server_len);
    memset(buffer, '\0', sizeof(buffer));

    // Si impostano le informazioni del server, nel momento in cui viene fatta la bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Creazione del socket.
    // Dato che upd è il protocollo di default per SOCK_DGRAM, il terzo parametro può essere 0 e verrà usato udp
    // sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error while creating socket");
        return EXIT_FAILURE;
    }

    // Il server effettua la bind per ricevere i messaggi dal client
    if (bind(sockfd, (struct sockaddr *)&server_addr, server_len) < 0)
    {
        perror("Error while binding socket");
        return EXIT_FAILURE;
    }

    while (1)
    {
        // Si riceve il messaggio dal client (o da un altro server di routing)
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client_addr, &client_len) < 0)
        {
            perror("Error while receiving server's msg");
            return EXIT_FAILURE;
        }

        printf("Ho ricevuto un messaggio da %s\n", inet_ntoa(client_addr.sin_addr));

        // Si processa il messaggio ricevuto, separando l'indirizzo IP del mittente dal messaggio
        // e il messaggio stesso, spezzando la stringa
        char buffer_tmp[INET_ADDRSTRLEN + MAX_MSG_SIZE + 1];
        strncpy(buffer_tmp, buffer, sizeof(buffer_tmp));
        char *ip = strtok(buffer_tmp, " ");

        if (is_ip_in_lan(client_addr.sin_addr.s_addr) && !is_ip_in_lan(inet_addr(ip)))
        {
            printf("Ho ricevuto un messaggio per %s da un client nella mia rete.\nNon conosco il destinatario, quindi lo invio agli altri server\n", ip);
            for (int i = 0; i < num_servers; i++)
            {
                // Si invia il messaggio a tutti gli altri server di routing
                other_server_addr.sin_family = AF_INET;
                other_server_addr.sin_port = htons(atoi(argv[1]));
                other_server_addr.sin_addr.s_addr = inet_addr(server_ips[i]);
                if (sendto(sockfd, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&other_server_addr, server_len) < 0)
                {
                    perror("Error while sending msg to other server");
                    return EXIT_FAILURE;
                }
            }
        }
        else if (is_ip_in_lan(inet_addr(ip)))
        {
            // Si invia il messaggio al client destinatario
            printf("Ho ricevuto un messaggio per %s, che è nella mia rete.\nLo inoltro al client\n", ip);
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(atoi(argv[2]));
            server_addr.sin_addr.s_addr = inet_addr(ip);
            if (sendto(sockfd, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&server_addr, server_len) < 0)
            {
                perror("Error while sending msg to client");
                return EXIT_FAILURE;
            }
        }
        else
        {
            printf("Questo server non conosce la macchina %s\nArresto del routing\n", ip);
        }
    }
    // Si chiude il socket
    close(sockfd);
    return EXIT_SUCCESS;
}