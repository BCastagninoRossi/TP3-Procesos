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

    while (1) 
    {
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
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        /* You should start programming from here... */
        int PID[command_count];

        char * command_list [command_count][MAX_COMMANDS];
        for (int i = 0 ; i < command_count ; i++){
            char* token = strtok(commands[i], " ");
            int j = 0;
            while (token != NULL){
                if (token[0] == '"' && token[strlen(token)-1]=='"'){
                    token++;
                    token[strlen(token)-1] = '\0';
                }
                command_list[i][j++] = token;
                token = strtok(NULL, " ");
            }
            command_list[i][j] = NULL;
        }

        if (command_count == 1){
            execvp(command_list[0][0], command_list[0]);
            return 0;
        }

        int PIPES[command_count-1][2];
        for (int i = 0; i < (command_count-1); i++){
            if (pipe(PIPES[i])==-1){
                fprintf(stderr, "Error while creating pipes\n");
            }
        }

        for (int i = 0; i < command_count; i++) 
        {
            printf("Command %d: %s\n", i, commands[i]);
            switch (PID[i]==fork())
            {
            case -1:
                fprintf(stderr, "Error initializing fork()\n");
                return -1;

            case 0:
                if (i !=0){
                    if (dup2(PIPES[i-1][0], STDIN_FILENO) == -1){
                        return -1;
                    }
                    close(PIPES[i-1][0]);
                }
                if (i != command_count-1){
                    if (dup2(PIPES[i][0], STDOUT_FILENO) == -1){
                    return -1;
                    }
                    close(PIPES[i][1]);
                }
                for (int j = 0; j < command_count; j++){
                    close(PIPES[j][0]);
                    close(PIPES[j][1]);
                }
                execvp(command_list[i][0], command_list[i]);
                perror("execvp");
                return 0;
            default:
                wait(NULL);
            }
        }
        command_count = 0;
    }
    return 0;
}
