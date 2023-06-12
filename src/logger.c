#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "tcpIO.h"

#define PORT 5678

char buffer[MAX_RESPONSE_SIZE] = {0};

bool requesting = true;

// Handle SIGINT signal
void handleSigInt(int sig) {
    if (sig != SIGINT) {
        return;
    }
    requesting = false;
}

int main(int argc, char const *argv[]) {
    (void)signal(SIGINT, handleSigInt);

    int logger_socket = 0, valread, server_port, logger_port;
    struct sockaddr_in serv_address;
    char *ip_address;

    // Get ip address and port from the command line arguments
    if (argc == 4) {
        ip_address = (char *)argv[1];
        server_port = atoi(argv[2]);
        logger_port = atoi(argv[3]);
    } else {
        printf("Usage: ./logger <ip_address> <server_port> <client_port>\n");
        return -1;
    }

    if ((logger_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printError("Socket creation failed");
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = inet_addr(ip_address);
    serv_address.sin_port = htons(server_port);

    // Include client address information to the datagram
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(logger_port);

    // Create enter message fromm VISITOR_MESSAGE and ENTER_REQUEST
    char request_message[MAX_RESPONSE_SIZE] = {0};
    strcpy(request_message, LOGGER_MESSAGE);

    while (requesting) {
        strcpy(buffer, request_message);

        // Send request message to the server
        if (sendto(logger_socket, (const char *)buffer, strlen(buffer), 0,
                   (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
            printError("Sendto failed");
        }

        memset(buffer, 0, MAX_RESPONSE_SIZE);

        // Receive response from the server
        socklen_t serv_address_len = sizeof(serv_address);
        if ((valread = recvfrom(logger_socket, (char *)buffer, MAX_RESPONSE_SIZE, 0,
                                (struct sockaddr *)&serv_address, &serv_address_len)) < 0) {
            printError("Recvfrom failed");
        }

        printf("%s\n", buffer);

        // Clear buffer
        memset(buffer, 0, MAX_RESPONSE_SIZE);

        sleep(5);
    }

    printf("Exiting gallery.\n");

    return 0;
}
