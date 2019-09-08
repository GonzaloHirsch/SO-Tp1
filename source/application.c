#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#define SLAVE_COUNT 10
#define WRITE_END 1
#define READ_END 0

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

int * createSlaves(int count, int ** sp){
	char * executeCommandArgs[4] = {"./slaveProcess.c","",NULL,NULL};
	int slaves[count];
	int * pipesSlaveToMain[count];
	int * pipesMainToSlave[count];
	
	int pid, error;

	for (int i = 0; i < count; i++){
		int pipeToSlave[2];	// 0 --> read end, 1 --> write end
		int pipeToMain[2];	// 0 --> read end, 1 --> write end

		// Creo ambos pipes
		if (pipe(pipeToSlave) != 0){ perror("Error: "); }
		if (pipe(pipeToMain) != 0){ perror("Error: "); }

		// Guardo el pipe para despues
		pipesSlaveToMain[i] = pipeToMain;
		pipesMainToSlave[i] = pipeToSlave;

		pid = fork();

		// En este caso es el hijo
		if (pid == 0){

			// Cierro el READ end del pipe que va hacia main
			close(pipeToMain[READ_END]);

			// Cierro el WRITE end del pipe que va hacia el slave
			close(pipeToSlave[WRITE_END]);

			// Le pongo el WRITE end del pipe que va al main en el STDOUT del slave
			dup2( pipeToMain[WRITE_END], STDOUT_FILENO);

			// Le pongo el READ end del pipe que va al slave en el STDIN del slave
			dup2( pipeToSlave[READ_END], STDIN_FILENO);

			// Cierro el WRITE end del pipe que va hacia main
			close(pipeToMain[WRITE_END]);

			// Cierro el READ end del pipe que va hacia el slave
			close(pipeToSlave[READ_END]);

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

	(*sps) = (*slavePipes*);

	return slaves;
}