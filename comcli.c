#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define MAX_MSG_SIZE 256
#define SERVER_QUEUE_NAME "/test1"


int main() {

    char command[MAX_MSG_SIZE];
    mqd_t mq;
    struct mq_attr attr;

    const char *cs_pipe = "cs";
    const char *sc_pipe = "sc";

    // Create the named pipes if they don't exist
    mkfifo(cs_pipe, 0666);
    mkfifo(sc_pipe, 0666);

    mq = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("Client: mq_open");
        exit(EXIT_FAILURE);
    }

    char msg[MAX_MSG_SIZE];
    sprintf(msg, "%s %s", cs_pipe, sc_pipe);
    if (mq_send(mq, msg, strlen(msg) + 1, 0) == -1) {
        perror("Client: mq_send");
        exit(EXIT_FAILURE);
    }

    int sc_fd; 
    int cs_fd;

    if((cs_fd = open(cs_pipe, O_WRONLY))<0){
        perror("open fifo");
        exit(1);
    }
    
    if((sc_fd = open(sc_pipe, O_RDONLY))<0){
        perror("open fifo");
        exit(1);
    }

    printf("xxx\n");
    if (cs_fd == -1 || sc_fd == -1) {
        perror("Failed to open FIFOs");
        exit(EXIT_FAILURE);
    }

    printf("Enter commands (type 'quit' to exit):\n");
    while (fgets(command, MAX_MSG_SIZE, stdin) != NULL) {
        printf("\nHere: ");
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
    unlink(cs_pipe);
    unlink(sc_pipe);

    mq_close(mq);
    printf("Client quitting.\n");
    return EXIT_SUCCESS;
}
