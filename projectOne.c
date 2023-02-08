#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

 void signalHandler(int);

 int main(int argc, char* argv[]) {
    char input[8];
    int nodeCount, in, out, nodeNum = 0, prevNodeNum = -1, nextNodeNum = -1;

    pid_t pid;
    signal (SIGINT, signalHandler);

    printf("Enter number of nodes: ");
    fgets(input, sizeof(input), stdin);
    nodeCount = atoi(input);
    int (*pipesList)[nodeCount][2] = malloc(sizeof(int) * nodeCount * 2);


     if (nodeCount == 0) {
        printf("ERROR: Invalid node amount\n");
        exit(1);
    }

    printf("Main Process ID: %d\n", getpid());

    printf("Creating pipes...\n");
    // Create pipes for future nodes
    for (int i = 0; i < nodeCount; i++) {
        int fd[2];
        if (pipe(fd) < 0) {
            perror("Failed pipe creation\n");
            exit(1);
        }
        (*pipesList)[i][0] = fd[0];
        (*pipesList)[i][1] = fd[1];
    }

    for (int i = 0; i < nodeCount; i++) {
        printf("\tPipe: %d\t fd[0]: %d\t fd[1]: %d\n", i, (*pipesList)[i][0], (*pipesList)[i][1]);
    }

    printf("\nCreating nodes...\n");
    pid = 1;
    for (int i = 0; i < nodeCount; i++) {
        if (pid != 0) { // parent process

            if ((pid = fork()) < 0) {
                perror("Fork failure");
                exit(1);
            }
            nodeNum = i;
            prevNodeNum = i - 1;
            nextNodeNum = i + 1;
            // TODO: Make math pretty here instead
            if (prevNodeNum < 0) {
                prevNodeNum = nodeCount - 1;
            }
            if (nextNodeNum > nodeCount - 1) {
                nextNodeNum = 0;
            }
            // Assign in for this child
            in = (*pipesList)[prevNodeNum][1];
            // Assign out for this child
            out = (*pipesList)[nodeNum][0];
        } else { // child process
            break;
        }
    }
    if (pid == 0) {
        printf("\tNode %d Created:\n", nodeNum);
        printf("\t\tprevNodeNum: %d\t nextNodeNum: %d\n", prevNodeNum, nextNodeNum);
        printf("\t\tin: %d\t out: %d\n", in, out);
        exit(0);
    } else {
        sleep(1);
        // Use out here to send messages to the node loop
    }
    return 0;
}

void signalHandler (int sigNum) {
    printf (" received an interrupt.\n");
    sleep (1);
    printf ("time to exit\n");
    exit(0);
}
