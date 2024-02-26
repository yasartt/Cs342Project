#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define MAX_MSG_SIZE 256
#define SERVER_QUEUE_NAME "/test1"
#define CS_PIPE "/tmp/cs_pipe"
#define SC_PIPE "/tmp/sc_pipe"

int main() {
    char command[MAX_MSG_SIZE];
    mqd_t mq;
    struct mq_attr attr;

    // Create the client and server communication pipes
    mkfifo(CS_PIPE, 0660);
    mkfifo(SC_PIPE, 0660);

    mq = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("Client: mq_open");
        exit(EXIT_FAILURE);
    }

    char msg[MAX_MSG_SIZE];
    sprintf(msg, "%s %s", CS_PIPE, SC_PIPE);
    if (mq_send(mq, msg, strlen(msg) + 1, 0) == -1) {
        perror("Client: mq_send");
        exit(EXIT_FAILURE);
    }

    int sc_fd = open(SC_PIPE, O_RDONLY);
    int cs_fd = open(CS_PIPE, O_WRONLY);
    if (cs_fd == -1 || sc_fd == -1) {
        perror("Failed to open FIFOs");
        exit(EXIT_FAILURE);
    }

    printf("Enter commands (type 'quit' to exit):\n");
    while (fgets(command, MAX_MSG_SIZE, stdin) != NULL) {
        command[strcspn(command, "\n")] = '\0'; // Remove newline
        write(cs_fd, command, strlen(command) + 1);
        if (strcmp(command, "quit") == 0) {
            read(sc_fd, msg, MAX_MSG_SIZE);
            printf("Server response: %s\n", msg);
            break;
        }

        // Wait for server response
        read(sc_fd, msg, MAX_MSG_SIZE);
        printf("Server response: %s\n", msg);
    }

    close(cs_fd);
    close(sc_fd);
    unlink(CS_PIPE);
    unlink(SC_PIPE);

    mq_close(mq);
    printf("Client quitting.\n");
    return EXIT_SUCCESS;
}
