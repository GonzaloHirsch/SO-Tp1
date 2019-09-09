#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
// Semaphore includes
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SLAVE_COUNT 3
#define WRITE_END 1
#define READ_END 0

int createSlaves(int count,int slaves[],int pipesWriteSlave[],int pipesReadSlave[]);
void sendFiles(const char * directory);
void sendFile(int pid, char * file);

int main(int argc, char * argv[]){

	int slaves[SLAVE_COUNT];
	int pipesWriteSlave[SLAVE_COUNT];
	int pipesReadSlave[SLAVE_COUNT];

	// Verificamos que le hayan pasado algo de parametro
	if (argc == 2){

		createSlaves(SLAVE_COUNT,slaves,pipesWriteSlave, pipesReadSlave);

		//sendFiles(argv[1]);

	} else if (argc > 2){
		printf("Demasiados argumentos.");
	} else {
		printf("Se espera un argumento.");
	}

	return 0;
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

int createSlaves(int count,int slaves[],int pipesWriteSlave[],int pipesReadSlave[]){
	char * executeCommandArgs[4] = {"./slaveProcess","",NULL,NULL};
	
	int i,pid, error;

	for (i = 0; i < count; i++){
		int pipeToSlave[2];	// 0 --> read end, 1 --> write end
		int pipeToMain[2];	// 0 --> read end, 1 --> write end

		// Creo ambos pipes
		if (pipe(pipeToSlave) != 0){ perror("Error: "); }
		if (pipe(pipeToMain) != 0){ perror("Error: "); }

		pid = fork();
		if(-1 == pid){
			perror("Fallo al iniciar los slave en main:");
			return -1;
		}

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

			// Cierro el WRITE end del pipe que va hacia main
			close(pipeToMain[WRITE_END]);
			// Cierro el READ end del pipe que va hacia el slave
			close(pipeToSlave[READ_END]);

			//Guardamos los pipes que nos interesan para interactuar desde el application al main
			pipesReadSlave[i] = pipeToMain[READ_END];
			pipesWriteSlave[i] = pipeToMain[WRITE_END];

			//Guardamos el pid del los proceso esclavos en el orden que fueron creados.
			slaves[i] = pid;
		} 	
	}

	return 0;
}