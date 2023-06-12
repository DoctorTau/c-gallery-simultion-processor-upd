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
#include "tcpIO.h"

#define PORT 5678
#define VISITORS_LIMIT 50
#define PICTURES_LIMIT 10

int server_fd;
int gallery[NUMBER_OF_PICTURES] = {0}, visitors = 0;

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
    char buffer[1024] = {0};
    sprintf(buffer, "Gallery info:\n");
    strcat(bufferString, buffer);
    sprintf(buffer, "Visitors: %d\n", visitors);
    strcat(bufferString, buffer);
    for (int i = 0; i < NUMBER_OF_PICTURES; i++) {
        sprintf(buffer, "Picture %d: %d visitors\n", i + 1, gallery[i]);
        strcat(bufferString, buffer);
    }
}

int main(int argc, char *argv[]) {
    (void)signal(SIGINT, handleSigInt);

    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len;
    int addrlen = sizeof(server_address), port;
    char reader_buffer[1024] = {0}, writer_buffer[1024] = {0};

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
        // Clear the buffer
        memset(reader_buffer, 0, sizeof(reader_buffer));
        memset(writer_buffer, 0, sizeof(writer_buffer));
        // Receive message from client
        client_address_len = sizeof(client_address);
        size_t bytesReceived = recvfrom(server_fd, reader_buffer, sizeof(reader_buffer), 0,
                                        (struct sockaddr *)&client_address, &client_address_len);
        if (bytesReceived < 0) {
            perror("Error receiving message from client");
        }

        // Get the first word from the message
        char *messageType = strtok(reader_buffer, " ");

        // Clear the buffer

        // If the first word is LOGGER_MESSAGE, then the message is from the logger
        if (strcmp(messageType, LOGGER_MESSAGE) == 0) {
            printf("Received message from logger\n");
            galleryInfo(writer_buffer);
            if (sendto(server_fd, writer_buffer, strlen(reader_buffer), 0,
                       (struct sockaddr *)&client_address, client_address_len) < 0) {
                perror("Error sending message to client");
            }
        } else if (strcmp(messageType, VISITOR_MESSAGE) == 0) {
            printf("Received message from visitor\n");
            // Get the second word from the message
            messageType = strtok(NULL, " ");
            char *picture_number_str = strtok(NULL, " ");

            if (strcmp(messageType, ENTER_REQUEST) == 0) {
                if (picture_number_str == NULL) {
                    if (visitors < VISITORS_LIMIT) {
                        visitors++;
                        // Send response to client
                        strcpy(writer_buffer, ENTER_RESPONSE);

                        printf("Visitor entered\n");
                    } else {
                        // Send response to client
                        strcpy(writer_buffer, REJECT_MESSAGE);
                    }
                } else {
                    int picture_number = atoi(picture_number_str);
                    if (picture_number >= 0 && picture_number <= NUMBER_OF_PICTURES) {
                        if (gallery[picture_number] < PICTURES_LIMIT) {
                            gallery[picture_number]++;
                            // Send response to client
                            strcpy(writer_buffer, ENTER_RESPONSE);

                            printf("Visitor entered picture %d\n", picture_number + 1);
                        } else {
                            // Send response to client
                            strcpy(writer_buffer, REJECT_MESSAGE);
                        }
                    } else {
                        // Send response to client
                        strcpy(writer_buffer, REJECT_MESSAGE);
                    }
                }
            } else if (strcmp(messageType, EXIT_REQUEST) == 0) {
                if (picture_number_str == NULL) {
                    if (visitors > 0) {
                        visitors--;
                        // Send response to client
                        strcpy(writer_buffer, EXIT_RESPONSE);
                        printf("Visitor left\n");
                    } else {
                        // Send response to client
                        strcpy(writer_buffer, REJECT_MESSAGE);
                    }
                } else {
                    int picture_number = atoi(picture_number_str);
                    if (picture_number >= 0 && picture_number <= NUMBER_OF_PICTURES) {
                        gallery[picture_number]--;
                        // Send response to client
                        strcpy(writer_buffer, EXIT_RESPONSE);
                        printf("Visitor left picture %d\n", picture_number + 1);
                    } else {
                        // Send response to client
                        strcpy(writer_buffer, REJECT_MESSAGE);
                    }
                }
            }
            if (sendto(server_fd, writer_buffer, strlen(reader_buffer), 0,
                       (struct sockaddr *)&client_address, client_address_len) < 0) {
                perror("Error sending message to client");
            }
        }
    }
    close(server_fd);

    return 0;
}
