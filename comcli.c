#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>

#define MAX_MSG_SIZE 256

void send_command_to_server(const char *mq_name, const char *cs_pipe, const char *sc_pipe, const char *command, int wsize) {
    mqd_t mq;
    char message[MAX_MSG_SIZE];

    // Open the message queue
    mq = mq_open(mq_name, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("Client: mq_open");
        exit(EXIT_FAILURE);
    }

    // Prepare the message to send to the server
    snprintf(message, sizeof(message), "%s %s %s", cs_pipe, sc_pipe, command);

    // Send message to the server
    if (mq_send(mq, message, strlen(message) + 1, 0) == -1) {
        perror("Client: mq_send");
        exit(EXIT_FAILURE);
    }

    // Here you might want to handle communication over the named pipes
    // This part is left as an exercise for simplicity

    mq_close(mq);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <MQ_NAME> [-b COMFILE] [-s WSIZE]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *mq_name = argv[1];
    char *comfile = NULL;
    int wsize = 512; // Default write size

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            comfile = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            wsize = atoi(argv[++i]);
        }
    }

    // Here, set up the named pipes (FIFOs) and prepare to send commands to the server
    // For demonstration, we'll use placeholder names for the FIFOs
    char *cs_pipe = "client_to_server_fifo";
    char *sc_pipe = "server_to_client_fifo";

    if (comfile) {
        // Batch mode: Read commands from the COMFILE and send them to the server
        FILE *file = fopen(comfile, "r");
        if (!file) {
            perror("Failed to open command file");
            exit(EXIT_FAILURE);
        }

        char command[MAX_MSG_SIZE];
        while (fgets(command, sizeof(command), file) != NULL) {
            send_command_to_server(mq_name, cs_pipe, sc_pipe, command, wsize);
            // Here, handle the server's response if needed
        }

        fclose(file);
    } else {
        // Interactive mode: Read commands from the user and send them to the server
        char command[MAX_MSG_SIZE];
        printf("Enter commands (type 'exit' to quit):\n");
        while (1) {
            fgets(command, sizeof(command), stdin);
            if (strncmp(command, "exit", 4) == 0) {
                break;
            }
            send_command_to_server(mq_name, cs_pipe, sc_pipe, command, wsize);
            // Here, handle the server's response if needed
        }
    }

    // Cleanup and close any open resources

    return EXIT_SUCCESS;
}
