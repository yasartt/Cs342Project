#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mqueue.h>

#define MAX_MSG_SIZE 256
#define SERVER_QUEUE_NAME "/test1"
#define QUEUE_PERMISSIONS 0660

void execute_client_request(const char *cs_pipe, const char *sc_pipe) {
    printf("Executea girdi");
    int cs_fd = open(cs_pipe, O_RDONLY);
    int sc_fd = open(sc_pipe, O_WRONLY);
    if (cs_fd == -1 || sc_fd == -1) {
        perror("Failed to open FIFOs");
        exit(EXIT_FAILURE);
    }

    char command[MAX_MSG_SIZE];
    while (1) {
        ssize_t bytes_read = read(cs_fd, command, MAX_MSG_SIZE);
        if (bytes_read > 0) {
            command[bytes_read] = '\0'; // Ensure null-termination
            printf("Received command: %s\n", command); // Echo command for demonstration
            if (strcmp(command, "quit") == 0) {
                write(sc_fd, "Server quitting.", 16);
                break;
            }
            // Respond back with command execution status (simplified)
            write(sc_fd, "Command executed", 17);
        }
    }

    close(cs_fd);
    close(sc_fd);
}

int main() {
    printf("First");

    struct mq_attr attr;
    char buffer[MAX_MSG_SIZE + 1]; // +1 for null terminator

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open("/test1", O_CREAT | O_RDONLY, QUEUE_PERMISSIONS, &attr);    if (mq == (mqd_t)-1) {
        perror("Server: mq_open");
        exit(EXIT_FAILURE);
    }

    printf("Server is running.\n");

    while (1) {
        printf("Second");
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_MSG_SIZE, NULL);
        if (bytes_read <= 0) {
            continue;
        }
        printf("Third");
        buffer[bytes_read] = '\0';
        printf("Received message: %s\n", buffer);

        char cs_pipe[MAX_MSG_SIZE], sc_pipe[MAX_MSG_SIZE];
        sscanf(buffer, "%s %s", cs_pipe, sc_pipe);

        pid_t pid = fork();
        if (pid == 0) { // Child process
            execute_client_request(cs_pipe, sc_pipe);
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            // Optionally wait for the child process to prevent zombies, or handle SIGCHLD.
            //int status;
            //waitpid(pid, &status, 0);
            continue;
        } else {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }

    mq_close(mq);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("Server quitting.\n");
    return EXIT_SUCCESS;
}
