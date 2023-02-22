#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

typedef struct packet {
    int dstNum;
    char data[255];
} packet;

void signalHandler(int);

int main(int argc, char *argv[]) {
    char input[255];
    int nodeCount;
    int in, out;
    int nodeNum = 0;
    int prevNodeNum = -1;
    int nextNodeNum = -1;

    pid_t pid;
    signal(SIGINT, signalHandler);

    // Get number of nodes
    printf("[MAIN] Enter number of nodes: ");
    fgets(input, sizeof(input), stdin);
    nodeCount = atoi(input);

    if (nodeCount == 0) {
        printf("ERROR: Invalid node amount\n");
        exit(1);
    }


    // Create an array of pipes
    int (*pipesList)[nodeCount][2] = malloc(sizeof(int) * nodeCount * 2);


    // Create pipes for future nodes
    printf("Creating pipes...\n");
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

            // Create a new Node
            if ((pid = fork()) < 0) {
                perror("Fork failure");
                exit(1);
            }
            nodeNum = i;

            // Fix for wrapping last node to first node
            prevNodeNum = i - 1;
            nextNodeNum = i + 1;
            if (prevNodeNum < 0) {
                prevNodeNum = nodeCount - 1;
            }
            if (nextNodeNum > nodeCount - 1) {
                nextNodeNum = 0;
            }

            // Assign in for this child
            in = (*pipesList)[prevNodeNum][0];

            // Assign out for this child
            out = (*pipesList)[nodeNum][1];

        } else { // child process leaves
            break;
        }
    }

    if (pid == 0) {
        // Debug info
        printf("\tNode %d Created:\n", nodeNum);
        printf("\t\tprevNodeNum: %d\t nextNodeNum: %d\n", prevNodeNum, nextNodeNum);
        printf("\t\tin: %d\t out: %d\n", in, out);
        do {
            struct packet pkt;

            // Waiting for data on the pipe
            if (read(in, &pkt, sizeof(struct packet)) < 0) {
                perror("Unable to read on pipe");
                exit(1);
            }

            if (pkt.dstNum == nodeNum) { // Check if data is for self
                printf("[NODE %d] Received data <%s>\n", nodeNum, pkt.data);
            } else { // If not, forward data to next node
                printf("[NODE %d] Received data <%s> forwarding to node %d\n", nodeNum, pkt.data, nodeNum + 1);
                if (write(out, &pkt, sizeof(struct packet)) < 0) {
                    perror("Unable to write on pipe");
                    exit(1);
                }
            }
        } while (1);
    } else {
        usleep(500000);
        printf("Main pipe: in= %d\tout=%d\n\n", in, out);
        // Use out here to send messages to the node loop
        while (1) {
            struct packet pkt;
            // Get user message
            printf("[MAIN] Input a message: ");
            fgets(input, sizeof(input), stdin);
            input[strlen(input) - 1] = '\0';
            strcpy(pkt.data, input);

            // Get destination node
            printf("[MAIN] Input a destination node: ");
            fgets(input, sizeof(input), stdin);
            pkt.dstNum = atoi(input);
            if (pkt.dstNum > nodeNum) {
                printf("[MAIN] Destination %d does not exist. Destination set to node 0\n", pkt.dstNum);
                pkt.dstNum = 0;
            }

            // Send packet to first node
            printf("[MAIN] Sending data <%s> to node %d\n", pkt.data, pkt.dstNum);
            write(out, &pkt, sizeof(struct packet));

            // Wait to allow corresponding node to print input and prevent gross output
            usleep(500000);
        }
    }
}

void signalHandler(int sigNum) {
    printf("[INTERRUPT] Closing process\n");
    exit(0);
}
