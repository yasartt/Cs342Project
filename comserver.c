#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <mqueue.h>

#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE + 10)

void handle_client(const char *cs_pipe, const char *sc_pipe) {
    // This function is supposed to open the FIFOs and handle the client-server communication
    printf("Handling client with CS_PIPE: %s and SC_PIPE: %s\n", cs_pipe, sc_pipe);
    // Here you would add the logic to open the FIFOs, execute commands, and send back responses
    // This part is left as an exercise
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <MQ_NAME>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *mq_name = argv[1];
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MSG_BUFFER_SIZE];

    // Setting up message queue attributes
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // Creating the message queue
    mq = mq_open(mq_name, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("Message queue creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections...\n");

    while (1) {
        ssize_t bytes_read = mq_receive(mq, buffer, MSG_BUFFER_SIZE, NULL);
        if (bytes_read <= 0) {
            perror("mq_receive");
            continue;
        }

        buffer[bytes_read] = '\0'; // Null-terminate the string

        // Simulating the reception of FIFO names via message queue for demonstration purposes
        printf("Received connection request: %s\n", buffer);

        // Forking a new process to handle this client
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Assuming the buffer contains the names of the FIFOs separated by a space
            char cs_pipe[100], sc_pipe[100];
            sscanf(buffer, "%s %s", cs_pipe, sc_pipe); // Extract FIFO names
            handle_client(cs_pipe, sc_pipe);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process: wait for the child to finish
            waitpid(pid, NULL, 0);
        }
    }

    // Cleanup
    mq_close(mq);
    mq_unlink(mq_name);

    return EXIT_SUCCESS;
}
