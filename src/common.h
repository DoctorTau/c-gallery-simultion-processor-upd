#ifndef COMMON_H
#define COMMON_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/wait.h>

void printError(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

#define NUMBER_OF_PICTURES 5
#define EXIT_MESSAGE "exit"
#define WELCOME_MESSAGE "Welcome to the gallery!\n"

#define VISITOR_MESSAGE "Visitor"
#define LOGGER_MESSAGE "Logger"

#endif
