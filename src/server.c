#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"
#include "queue.h"
#include "vector.h"
#include "tcpIO.h"

#define PORT 5678

int server_fd;

void closeConnection() {
    close(server_fd);
}

void handleSigInt(int sig) {
    if (sig != SIGINT) {
        return;
    }
    closeConnection();
    // kill warder process
    exit(EXIT_SUCCESS);
}

void galleryInfo(char *bufferString) {
}

int main(int argc, char *argv[]) {
    (void)signal(SIGINT, handleSigInt);

    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;
    int addrlen = sizeof(server_address), port;
    char buffer[1024] = {0};

    // Get port from the command line arguments
    if (argc == 2) {
        port = atoi(argv[1]);
    } else {
        printf("Usage: ./server <port>\n");
        return -1;
    }

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind the socket to the specified IP address and port
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Binding failed");
        closeConnection();
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", port);

    for (;;) {
        // Receive message from client
        client_address_len = sizeof(client_address);
        size_t bytesReceived = recvfrom(server_fd, buffer, sizeof(buffer), 0,
                                        (struct sockaddr *)&client_address, &client_address_len);
        if (bytesReceived < 0) {
            perror("Error receiving message from client");
        }

        buffer[bytesReceived] = '\0';

        // Print message from client
        printf("Received message from client: %s\n", buffer);

        // Write another message to client "Hello from server"
        strcpy(buffer, "Hello from server");

        // Send message to client
        if (sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_address,
                   client_address_len) < 0) {
            perror("Error sending message to client");
        }
    }

    close(server_fd);

    return 0;
}
