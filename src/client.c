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
bool pictures[NUMBER_OF_PICTURES] = {false};

int getRandomNumber(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

bool isAllPicturesVisited() {
    for (int i = 0; i < NUMBER_OF_PICTURES; i++) {
        if (!pictures[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Returns a random picture number that has not been visited yet.
 * If all pictures have been visited, returns -1.
 *
 * @return int - the next random picture number or -1 if all pictures have been visited
 */
int getNextRandomPicture() {
    if (isAllPicturesVisited()) {
        return -1;
    }
    int picture = getRandomNumber(0, NUMBER_OF_PICTURES - 1);
    while (pictures[picture]) {
        picture = getRandomNumber(0, NUMBER_OF_PICTURES - 1);
    }
    pictures[picture] = true;
    return picture;
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
    client_address.sin_port = htons(client_port);

    // Create enter message fromm VISITOR_MESSAGE and ENTER_REQUEST
    char enterMessage[MAX_RESPONSE_SIZE] = {0};
    strcpy(enterMessage, VISITOR_MESSAGE);
    strcat(enterMessage, " ");
    strcat(enterMessage, ENTER_REQUEST);

    for (;;) {
        // Write hello string to buffer
        strcpy(buffer, enterMessage);

        if (sendto(client_socket, (const char *)buffer, strlen(buffer), 0,
                   (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
            printError("Sendto failed");
        }

        printf("Request sent.\n");

        // Clear buffer
        memset(buffer, 0, MAX_RESPONSE_SIZE);

        // Receive welcome message from server
        socklen_t serv_address_len = sizeof(serv_address);
        if ((valread = recvfrom(client_socket, (char *)buffer, MAX_RESPONSE_SIZE, 0,
                                (struct sockaddr *)&serv_address, &serv_address_len)) < 0) {
            printError("Recvfrom failed");
        }

        printf("Resived: %s\n", buffer);

        if (strcmp(buffer, ENTER_RESPONSE) == 0) {
            printf("Entering the gallery.\n");
            break;
        }
    }

    int picture = getNextRandomPicture();
    while (picture != -1) {
        char picture_request[MAX_RESPONSE_SIZE] = {0};
        strcpy(picture_request, VISITOR_MESSAGE);
        // Add picture number to the request
        char picture_number[2];
        sprintf(picture_number, "%d", picture);
        strcat(picture_request, " ");
        strcat(picture_request, ENTER_REQUEST);
        strcat(picture_request, " ");
        strcat(picture_request, picture_number);

        for (;;) {

            // Write picture request to buffer
            strcpy(buffer, picture_request);

            if (sendto(client_socket, (const char *)buffer, strlen(buffer), 0,
                       (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
                printError("Sendto failed");
            }

            printf("Request for picture %d sent.\n", picture + 1);

            // Clear buffer
            memset(buffer, 0, MAX_RESPONSE_SIZE);

            // Receive message from server
            socklen_t serv_address_len = sizeof(serv_address);
            if ((valread = recvfrom(client_socket, (char *)buffer, MAX_RESPONSE_SIZE, 0,
                                    (struct sockaddr *)&serv_address, &serv_address_len)) < 0) {
                printError("Recvfrom failed");
            }

            if (strcmp(buffer, ENTER_RESPONSE) == 0) {
                printf("Entering picture %d.\n", picture + 1);
                sleep(5);

                // Create exit message fromm VISITOR_MESSAGE and EXIT_REQUEST
                char exitMessage[MAX_RESPONSE_SIZE] = {0};
                strcpy(exitMessage, VISITOR_MESSAGE);
                strcat(exitMessage, " ");
                strcat(exitMessage, EXIT_REQUEST);
                strcat(exitMessage, " ");
                strcat(exitMessage, picture_number);

                memset(buffer, 0, MAX_RESPONSE_SIZE);
                strcpy(buffer, exitMessage);
                if (sendto(client_socket, (const char *)buffer, strlen(buffer), 0,
                           (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
                    printError("Sendto failed");
                }
                break;
            }
        }

        picture = getNextRandomPicture();
    }

    // Create exit message fromm VISITOR_MESSAGE and EXIT_REQUEST
    char exitMessage[MAX_RESPONSE_SIZE] = {0};
    strcpy(exitMessage, VISITOR_MESSAGE);
    strcat(exitMessage, " ");
    strcat(exitMessage, EXIT_REQUEST);

    strcpy(buffer, exitMessage);
    if (sendto(client_socket, (const char *)buffer, strlen(buffer), 0,
               (const struct sockaddr *)&serv_address, sizeof(serv_address)) < 0) {
        printError("Sendto failed");
    }

    printf("Exiting gallery.\n");

    return 0;
}
