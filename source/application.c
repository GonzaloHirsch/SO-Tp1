#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#define SLAVE_COUNT 10

int main(int argc, char * argv[]){

	int * slaves;
	int ** slavePipes;

	// Verificamos que le hayan pasado algo de parametro
	if (argc == 2){

		slaves = createSlaves(SLAVE_COUNT, slavePipes);

		sendFiles(argv[1]);

	} else if (argc > 2){
		printf("Demasiados argumentos.");
	} else {
		printf("Se espera un argumento.");
	}
}

void sendFile(int pid, char * file){

}

void sendFiles(const char * directory){
	// Puntero al directorio
	DIR * d;

	// Struct para lo que devuelve el readdir
    struct dirent *dir;

    // Apertura del directorio
    d = opendir(directory);

    // Valido que no haya habido ningun error al abrirlo
    if (d != NULL)
    {
    	// Mientras hayan archivos en el directorio
        while ((dir = readdir(d)) != NULL)
        {
        	// Verifico que sea un archivo lo que se leyo en el directorio
        	if (dir->d_type == DT_REG){

        	}
        }

        // Cerra el archivo cuando terminamos
        closedir(d);
    } 
    // Si es null, se termina la ejecucion del programa y muestra el error correspondiente
    else {
    	perror("Error: ");
    }
}

int * createSlaves(int count, int ** sps){
	char * executeCommandArgs[4] = {"./slaveProcess.c","",NULL,NULL};
	int slaves[count];
	int * slavePipes[count];
	
	int pid, error;

	for (int i = 0; i < count; i++){
		int slavePipe[2];	// 0 --> read end, 1 --> write end

		if (pipe(slavePipe) != 0){
			perror("Error: ");
		}

		// Guardo el pipe para despues
		slavePipes[i] = slavePipe;

		pid = fork();

		// En este caso es el hijo
		if (pid == 0){
			error = execvp(executeCommandArgs[0],executeCommandArgs);
			if (error < 0){
				perror("Error: ");
			}
		} 
		// En este caso es el padre
		else if (pid > 0){
			// Guarda el PID del hijo en el array de slaves
			slaves[i] = pid;
		} 
	}

	sps = slavePipes;

	return slaves;
}