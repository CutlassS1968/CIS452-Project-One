#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

 void signalHandler(int);

 int main(int argc, char* argv[]) {
    char input[8];
    int nodeCount, nodeNum = 0;
    pid_t pid;
    signal (SIGINT, signalHandler);

    printf("Enter number of nodes: ");
    fgets(input, sizeof(input), stdin);
    nodeCount = atoi(input);

    // Create pipe overhead
    int *childPipes[nodeCount];

    if (nodeCount == 0) {
        printf("ERROR: Invalid node amount\n");
        exit(1);
    }

    printf("ppid: %d\n", getpid());
    pid = 1;
    // TODO: Creating incorrect number of nodes
    for (int i = 0; i < nodeCount; i++) {
        if (pid != 0) { // parent process
            int fd[2];
            int pipeCreationResult;
            pipeCreationResult = pipe(fd);
            if (pipeCreationResult < 0) {
                perror("Failed pipe creation\n");
                exit(1);
            }
            childPipes[i] = fd;
            if ((pid = fork()) < 0) {
                perror("Fork failure");
                exit(1);
            }
            nodeNum = i;
        } else { // child process
            break;
        }
    }
    if (pid == 0) {
        printf("nodeNum: %d\n", nodeNum);
        exit(0);
    }
    return 0;
}

void signalHandler (int sigNum) {
    printf (" received an interrupt.\n");
    sleep (1);
    printf ("time to exit\n");
    exit(0);
}
