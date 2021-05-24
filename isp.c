#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h> 
#include <ctype.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>

#define MAX_INPUT_SIZE 256
#define MAX_WORD_SIZE 256
#define MAX_NUMBER_OF_WORDS 256

#define READ_END 0
#define WRITE_END 1

#define NORMAL_MODE 1
#define TAPPED_MODE 2

int main(int argc, char *argv[]) {
    
    const int N = atoi(argv[1]);
    const int mode = atoi(argv[2]);

    do {
        char inputCommand[MAX_INPUT_SIZE]; 
        char tokens[MAX_NUMBER_OF_WORDS][MAX_WORD_SIZE];
        int counter;
        int pipePosition = -1;
        // For normal mode
        int fd[2]; 

        // For tapped mode
        int fd1[2];
        int fd2[2];

        // To measure Time
        struct timeval t1, t2;
        // Display prompt and get input
        printf("%s", "isp$ ");         
        scanf("%[^\n]%*c", inputCommand); 

        // Extracting tokens from input string
        char *token = strtok(inputCommand, " ");
        counter = 0;
        pipePosition = -1;
        int firstChildArgsLength = 0, secondChildArgsLength = -1; // -1 because pipe token position should not be included 
        while (token != NULL) {
            strcpy(tokens[counter], token);
            if ((strcmp("|", token) == 0) && (pipePosition == -1)) {
                pipePosition = counter;
            }
            if (pipePosition == -1) firstChildArgsLength++;
            else secondChildArgsLength++; 
            counter++;
            token = strtok(NULL, " ");
        } 

        // // Arguments for first child
        char *argumentsForFirstChild[firstChildArgsLength + 1];
        for (int i = 0; i < firstChildArgsLength; i++) {
            argumentsForFirstChild[i] = tokens[i];
        }
        argumentsForFirstChild[firstChildArgsLength] = NULL;
        
        // // Arguments for second child
        char *argumentsForSecondChild[secondChildArgsLength + 1];
        if (pipePosition != -1) {
            int startPosition = pipePosition + 1;
            for (int i = 0; i < secondChildArgsLength; i++) {
                argumentsForSecondChild[i] = tokens[startPosition];
                startPosition++;
            }
        }
        argumentsForSecondChild[secondChildArgsLength] = NULL;
        
        // Single command
        if (pipePosition == -1) {
            pid_t pidFirstChild = fork();
            if (pidFirstChild < 0) {
                printf("%s\n", "Fork failed.");
                exit(1);
            } else if (pidFirstChild == 0) {
                exit(execvp(argumentsForFirstChild[0], argumentsForFirstChild));
            } else {
                wait(NULL);
            }
        } else { // 2 Commands
            if (mode == NORMAL_MODE) {
                // gettimeofday(&t1, NULL);
                // Create Unnamed Pipe
                if (pipe(fd) == -1) {
                    printf("%s\n", "Pipe failed.");
                    exit(1);
                }
                // Execution
                pid_t pidFirstChild = fork();
                if (pidFirstChild < 0) {
                    printf("%s\n", "Fork failed.");
                    exit(1);
                } else if (pidFirstChild == 0) { // first child executing
                    close(STDOUT_FILENO);  
                    dup2(fd[WRITE_END], WRITE_END);         
                    close(fd[READ_END]);       
                    close(fd[WRITE_END]);
                    exit(execvp(argumentsForFirstChild[0], argumentsForFirstChild));
                }
                pid_t pidSecondChild = fork();
                if (pidFirstChild < 0) {
                    printf("%s\n", "Fork failed.");
                    exit(1);
                } else if (pidSecondChild == 0) { // second child executing
                    close(STDIN_FILENO);   
                    dup2(fd[READ_END], READ_END);
                    close(fd[WRITE_END]);
                    close(fd[READ_END]);
                    exit(execvp(argumentsForSecondChild[0], argumentsForSecondChild));
                }
                close(fd[READ_END]);
                close(fd[WRITE_END]);
                wait(NULL);
                wait(NULL);
                // gettimeofday(&t2, NULL);
            } else if (mode == TAPPED_MODE) {
                int charCount = 0;
                int readCount = 0;
                int writeCount = 0;
                // gettimeofday(&t1, NULL);
                // Parent
                pipe(fd1);
                pipe(fd2);
                pid_t pidFirstChild = fork();
                if (pidFirstChild < 0) {
                    printf("%s\n", "Fork failed.");
                    exit(1);
                } else if (pidFirstChild == 0) {
                    dup2(fd1[WRITE_END], WRITE_END);
                    close(fd1[READ_END]);
                    close(fd1[WRITE_END]);
                    close(fd2[READ_END]);
                    close(fd2[WRITE_END]);
                    exit(execvp(argumentsForFirstChild[0], argumentsForFirstChild));
                } else {
                    pid_t pidSecondChild = fork();
                    if (pidSecondChild == 0) {
                        dup2(fd2[READ_END], READ_END);
                        close(fd1[READ_END]);
                        close(fd1[WRITE_END]);
                        close(fd2[READ_END]);
                        close(fd2[WRITE_END]);
                        exit(execvp(argumentsForSecondChild[0], argumentsForSecondChild));
                    } 
                    // Read from first pipe and write into second pipe
                    char message[N];
                    int numOfBytesRead;
                    close(fd1[WRITE_END]);
                    close(fd2[READ_END]);
                    int numOfBytesWritten = 0;
                    while (numOfBytesRead = read(fd1[READ_END], message, N)) {
                        numOfBytesWritten = write(fd2[WRITE_END], message, numOfBytesRead);
                        readCount++;
                        writeCount++;
                        charCount += numOfBytesRead;
                    }
                    close(fd1[READ_END]);
                    close(fd2[WRITE_END]);
                    wait(NULL);
                    wait(NULL);
                    // gettimeofday(&t2, NULL);
                    printf("%s", "\n");
                    printf("%s%d\n", "character-count: ", charCount);
                    printf("%s%d\n", "read-call-count: ", readCount);
                    printf("%s%d\n", "write-call-count: ", writeCount);
                    printf("%s", "\n");
                }
            } else { 
                printf("%s\n", "Unimplemented mode.");
                exit(1);
            }
            printf("%f ms.\n", (t2.tv_usec - t1.tv_usec) / 1000.0);
        }
    } while (1);

    return 0;
}