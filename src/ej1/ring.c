#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*create children function*/
void children (int n, int (*pipes)[2], int buffer[1], int start, int current){
	if (current == start){
		int value = buffer[0]+1;
		write(pipes[(current+1)%n][1], &value, sizeof(int));
		printf("Proceso %i envía %i \n", current, value);
	}else{
		int value;
		read(pipes[current][0], &value, sizeof(int));
		value +=1;
		write(pipes[(current+1)%n][1], &value, sizeof(int));
		printf("Proceso %i recibe %i  y envía %i \n", current, value-1, value);
		}
}

/*create parent function*/
int parent (int (*pipes)[2], int start){
	int value;
	read(pipes[start][0], &value, sizeof(int));
	printf("El proceso padre recibió %i \n", value);
	return 0;

}
int main(int argc, char **argv)
{	
	int start, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}

    /* Parsing of arguments */
	start = atoi(argv[3]);
	n = atoi(argv[1]);
	buffer[0] = atoi(argv[2]);
    
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
	/*Create pipes*/
	int fd[n][2];
	for (int i = 0; i < n; i++){
		if (pipe(fd[i]) == -1){
			perror("Error creating pipe");
			return -1;
		}
	}

	int PID[n];
	for (int i = 0; i < n; i++) {
		int curr = (i + start)%n;
		PID[i] = fork();
		if (PID[i] == 0) {
			children(n, fd, buffer, start, curr);
			return 0;
		}
	}
	for (int i = 0; i < n; i++) {
		waitpid(PID[i], NULL, 0);
	}
	parent(fd, start);
}