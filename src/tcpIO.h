#ifndef TCPIO_H
#define TCPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_RESPONSE_SIZE 1024

void SendMessage(int sock, const char *message) {
    // printf("Sending message: %s\nTo: %d\n", message, sock);

    if (send(sock, message, MAX_RESPONSE_SIZE, 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

void ReceiveMessage(int sock, char *buffer) {
    // Clear buffer
    memset(buffer, 0, MAX_RESPONSE_SIZE);

    // printf("Receiving message from: %d\n", sock);
    if (read(sock, buffer, MAX_RESPONSE_SIZE) < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
}

#endif  // TCPIO_H
