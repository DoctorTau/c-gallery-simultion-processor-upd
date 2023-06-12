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

    int sock = 0, valread, port;
    struct sockaddr_in serv_addr;
    char *ip_address;

    // Get ip address and port from the command line arguments
    if (argc == 3) {
        ip_address = (char *)argv[1];
        port = atoi(argv[2]);
    } else {
        printf("Usage: ./client <ip_address> <port>\n");
        return -1;
    }

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    sleep(1);

    SendMessage(sock, LOGGER_MESSAGE);

    // Read the message from the server to enter the gallery
    ReceiveMessage(sock, buffer);

    if (buffer == WELCOME_MESSAGE) {
        printf("Entering the gallery.\n");
    }

    while (requesting) {
        // Send message with log request
        SendMessage(sock, "Log request");

        // Read the message from the server
        ReceiveMessage(sock, buffer);

        // Clear console
        printf("\033[2J\033[1;1H");

        printf("[LOG]\n%s\n", buffer);

        sleep(5);
    }

    // Send the message to leave the gallery
    SendMessage(sock, EXIT_MESSAGE);
    printf("Exiting gallery.\n");

    return 0;
}
