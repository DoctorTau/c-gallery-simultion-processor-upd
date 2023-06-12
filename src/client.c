#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "common.h"
#include "tcpIO.h"

#define PORT 5678

char buffer[MAX_RESPONSE_SIZE] = {0};

int getRandomNumber(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

bool isAllTrue(bool array[]) {
    for (int i = 0; i < NUMBER_OF_PICTURES; i++) {
        if (!array[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char const *argv[]) {
    int client_socket = 0, valread, server_port, client_port;
    struct sockaddr_in serv_address;
    char *ip_address;

    // Get ip address and port from the command line arguments
    if (argc == 4) {
        ip_address = (char *)argv[1];
        server_port = atoi(argv[2]);
        client_port = atoi(argv[3]);
    } else {
        printf("Usage: ./client <ip_address> <server_port> <client_port>\n");
        return -1;
    }

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printError("Socket creation failed");
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = inet_addr(ip_address);
    serv_address.sin_port = htons(server_port);

    // Include client address information to the datagram
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_address.sin_port = htons(client_port);

    // Write hello string to buffer
    strcpy(buffer, "Hello from client!");

    if (sendto(client_socket, (const char *)buffer, strlen(buffer), 0,
               (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
        printError("Sendto failed");
    }

    printf("Hello message sent.\n");

    // Receive welcome message from server
    socklen_t serv_address_len = sizeof(serv_address);
    if ((valread = recvfrom(client_socket, (char *)buffer, MAX_RESPONSE_SIZE, 0,
                            (struct sockaddr *)&serv_address, &serv_address_len)) < 0) {
        printError("Recvfrom failed");
    }
    buffer[valread] = '\0';
    printf("%s\n", buffer);

    printf("Exiting gallery.\n");

    return 0;
}
