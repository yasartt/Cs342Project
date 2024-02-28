#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#define MAX_MSG_SIZE 256
#define SERVER_QUEUE_NAME "/test1"


int main(int argc, char *argv[]) {

    char command[MAX_MSG_SIZE];
    mqd_t mq;
    struct mq_attr attr;

    char *comFile = NULL;
    int wsize = MAX_MSG_SIZE; // Default write size
    int opt;

    // Parse command-line arguments for batch mode and write size
    while ((opt = getopt(argc, argv, "b:s:")) != -1) {
        switch (opt) {
            case 'b':
                comFile = optarg; // Batch file name
                break;
            case 's':
                wsize = atoi(optarg); // Write size limit
                if (wsize <= 0) {
                    fprintf(stderr, "Invalid WSIZE. Using default.\n");
                    wsize = 512;
                    exit(EXIT_FAILURE);
                }
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-b COMFILE] [-s WSIZE]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char cs_pipe[30], sc_pipe[30]; // Adjusted size for safety
    pid_t pid = getpid(); // Get the current process ID



    // Create unique FIFO names using the process ID
    snprintf(cs_pipe, sizeof(cs_pipe), "/tmp/cs_%d", pid);
    snprintf(sc_pipe, sizeof(sc_pipe), "/tmp/sc_%d", pid);

    // Create the named pipes (FIFOs)
    if (mkfifo(cs_pipe, 0666) < 0 ) {
        perror("Client: mkfifo CS_PIPE failed");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(sc_pipe, 0666) < 0 ) {
        perror("Client: mkfifo SC_PIPE failed");
        exit(EXIT_FAILURE);
    }

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

    if (cs_fd == -1 || sc_fd == -1) {
        perror("Failed to open FIFOs");
        exit(EXIT_FAILURE);
    }
    FILE *inputStream = stdin; // Default to stdin for interactive mode
    if (comFile != NULL) {
        inputStream = fopen(comFile, "r");
        if (inputStream == NULL) {
            perror("Client: Opening COMFILE failed");
            exit(EXIT_FAILURE);
        }
    }

    printf("Enter commands (type 'quit' to exit):\n");
    while (fgets(command, wsize, inputStream) != NULL) {
        command[strcspn(command, "\n")] = '\0'; // Remove newline
        if (write(cs_fd, command, wsize) < 0) {
            perror("Client: write to cs_pipe failed");
            break;
        }

        if (strcmp(command, "quit") == 0) {
            break; // Exit loop after sending "quit"
        }

        // Read and display server response
        if (read(sc_fd, msg, wsize) > 0) { // Note: Server needs to respect 'wsize' limit
            printf("Server response: %s\n", msg);
        }
    }

    if (comFile != NULL) {
        fclose(inputStream);
    }

    close(cs_fd);
    close(sc_fd);
    unlink(cs_pipe);
    unlink(sc_pipe);

    mq_close(mq);
    printf("Client quitting.\n");
    return EXIT_SUCCESS;
}