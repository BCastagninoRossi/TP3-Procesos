#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|"); //ls | grep ".zip"
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }
        
        // int *PIDS = malloc(sizeof(int) * command_count);
        // if(!PIDS){return -1;}

        int PIDS[command_count];
        
        // int **pipes = create_pipes(command_count - 1);
        int pipes[command_count - 1][2];
        for (int i = 0; i < command_count - 1; i++){
            if (pipe(pipes[i]) == -1){
                fprintf(stderr, "Error al crear los pipes\n");
                return -1;
            }
        }
        /* You should start programming from here... */
        char * command_matrix [command_count][MAX_COMMANDS];
        for (int i = 0; i < command_count; i++) {
            char *arg = strtok(commands[i], " ");
            int j = 0;
            while (arg != NULL) {
                if (arg[0] == '"' && arg[strlen(arg) - 1] == '"') {
                    arg++;
                    arg[strlen(arg) - 1] = '\0';
                }
                command_matrix[i][j++] = arg;
                arg = strtok(NULL, " ");
            }
            command_matrix[i][j] = NULL; 
        }

        for (int i = 0; i < command_count; i++) {
            printf("Comando %d: %s\n", i, commands[i]);
            switch (PIDS[i] = fork()) {
                case -1:
                    fprintf(stderr, "Error al hacer el fork()\n");
                    return -1;
                    exit(1);

                case 0:
                    if (i != 0) {
                        close(pipes[i - 1][1]);
                        if(dup2(pipes[i - 1][0], STDIN_FILENO) == -1){
                            return -1;
                        }
                        close(pipes[i - 1][0]);
                    }
                    if (i != command_count - 1) {
                        close(pipes[i][0]);
                        if(dup2(pipes[i][1], STDOUT_FILENO)  == -1){
                            return -1;
                        }
                        close(pipes[i][1]);
                    }
                    for (int j = 0; j < command_count -1; j++) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                    execvp(command_matrix[i][0], command_matrix[i]);
                    perror("execvp");
                    exit(0);
                default:
                    wait(NULL);
                    if (i != 0) {
                        close(pipes[i - 1][0]); // si no se queda esperando
                    }
                    if (i != command_count - 1) {
                        close(pipes[i][1]);
                    }
            }
        }
        command_count = 0; 
    }
    return 0;
}