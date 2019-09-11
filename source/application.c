#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/select.h>
// Semaphore includes
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SLAVE_COUNT 3
#define MAX_INFO_TO_SLAVE 300
#define MAX_INFO_FROM_SLAVE 200

#define WRITE_END 1
#define READ_END 0

#define FILE_DELIMITER " ";
#define FILE_ENDER "\n"

int createSlaves(int count,int slaves[],int pipesSlave[][2]);
void readInfoSlave(int pipesSlave[][2], int slaveNum);
void sendInfoSlave(int pipesSlave[][2],int slaveNum, char ** filesToProcess, int filesSend);
int sendInitialFiles(int fileCant, int pipesSlave[][2], char ** filesToProcess);
void terminateSlaves(int pipesSlave[][2]);



//TO DO: Agregar que cierre los pipes al final.
int main(int argc, char * argv[]){

	int filesSend = 0,filesCant = argc-1,filesRec =0  ;
	int slaves[SLAVE_COUNT];
	int pipesSlave[SLAVE_COUNT][2];
	//Los archivos a procesar empiezan con el 2do archivo de los argumentos.
	char ** filesToProcess = argv+1;

	//Creamos los esclavos 
	createSlaves(SLAVE_COUNT,slaves,pipesSlave);

	//Creamos el set de pipes a esperar para que lean.
	fd_set pipeReadSet;
	int i=0, numPipesReady;
	//Struct que indica el tiempo a max a esperar a recibir data.
	struct timeval tv;
	tv.tv_sec = 20;
    tv.tv_usec = 0;

	FD_ZERO(&pipeReadSet);
	for(i=0;i<SLAVE_COUNT;i++){
		FD_SET(pipesSlave[i][READ_END],&pipeReadSet);
	}

	//Distribuimos los archivos iniciales
	filesSend = sendInitialFiles(filesCant,pipesSlave,filesToProcess);

	while(filesRec < filesCant){
		//Esperaremos por un tiempo indeterminado que alguno de los pipes este listo para la lectura.

		numPipesReady = select(SLAVE_COUNT+1, &pipeReadSet, NULL,NULL, &tv);


		if(numPipesReady == -1){
			perror("Error in select():");
			break;
		}
		else if(numPipesReady == 0){
			perror("No data received");
			break;
		}

		//Buscamos en cada pipe de los esclavos cual de ellos recibio info.
		for(i=0;i<SLAVE_COUNT;i++){

			printf("%d %d\n", i,FD_ISSET(pipesSlave[i][READ_END],&pipeReadSet));

			//Si alguno recibio info...
			if(FD_ISSET(pipesSlave[i][READ_END],&pipeReadSet)){

				readInfoSlave(pipesSlave,i);//Leemos la informacion recibida
				filesRec++;

				//Si todavia quedan archivos por mandar le enviamos 1 mas.
				if(filesSend < filesCant){
					sendInfoSlave(pipesSlave,i,filesToProcess,filesSend);
					filesSend++;
				}
			}
				
		}
		
		
	}

	printf("Ending\n");
	terminateSlaves(pipesSlave);
	

	return 0;
}

int createSlaves(int count, int slaves[], int pipesSlave[][2]){
	char * executeCommandArgs[3] = {"./slaveProcess",NULL,NULL};
	
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
				perror("Error: fallo al ejecutar el slaveProcess");
				exit(0);
			}
		} 
		// En este caso es el padre
		else if (pid > 0){

			// Cierro el WRITE end del pipe que va hacia main
			close(pipeToMain[WRITE_END]);
			// Cierro el READ end del pipe que va hacia el slave
			close(pipeToSlave[READ_END]);

			//Guardamos los pipes que nos interesan para interactuar desde el application al slave
			pipesSlave[i][READ_END] = pipeToMain[READ_END];
			pipesSlave[i][WRITE_END] = pipeToSlave[WRITE_END];

			//Guardamos el pid del los proceso esclavos en el orden que fueron creados.
			slaves[i] = pid;
		} 	
	}

	return 0;
}

void readInfoSlave(int pipesSlave[][2], int slaveNum){

    char tempBuffer[MAX_INFO_FROM_SLAVE], c = 0;
    int cnt = 0;

	//Leemos del pipe hasta que termine la string enviada
    while(read(pipesSlave[slaveNum][READ_END], &c, 1) != 0 && cnt < MAX_INFO_FROM_SLAVE - 1){
        if(c == '\n'){
            tempBuffer[cnt] = 0;
            break;
        }
        tempBuffer[cnt++] = c;
    }
    tempBuffer[cnt] = 0; //Por las dudas, hay que asegurar que termine con 0.


	//Por ahora solo lo imprimimos, espero a que ribas me diga como guardar en shm
	printf("%s\n", tempBuffer);
}

void sendInfoSlave(int pipesSlave[][2],int slaveNum, char ** filesToProcess, int filesSend){
	write(pipesSlave[slaveNum][WRITE_END], filesToProcess[filesSend],strlen(filesToProcess[filesSend]));
	write(pipesSlave[slaveNum][WRITE_END],"\n",1);
}

/*
	Send initial files to be distributed. 
	Returns the files already distributed.
 */
int sendInitialFiles(int filesCant, int pipesSlave[][2], char ** filesToProcess){
	//Take half of the files and distribute an equal quantity to each slave.

	int filesPerSlave = (filesCant/2)/SLAVE_COUNT;
	int filesSend = 0, slaveNum ,i;

	//Caso especial: Si inicialmente tenemos mas esclavos que la mitad de archivos a procesar
	//le repartimos 1 a cada uno.
	if(filesPerSlave == 0)
		filesPerSlave = 1;
	
	for(slaveNum=0;slaveNum < SLAVE_COUNT;slaveNum++){
		
		//Iteramos por cada esclavo repartiendole la cantidad de archivos apropiada.
		//Caso especial: fileCount<fileNums por si tenemos mas esclavos que archivos a procesar.

		for(i=0;i<filesPerSlave && filesSend < filesCant;i++){

			write(pipesSlave[slaveNum][WRITE_END], filesToProcess[filesSend],strlen(filesToProcess[filesSend]));
			if(i+1 != filesPerSlave)//No queremos un espacio en el ultimo...
				write(pipesSlave[slaveNum][WRITE_END]," ",1);
			else{//Si es el ultimo lo terminamos con \n
				write(pipesSlave[slaveNum][WRITE_END],"\n",1);
			}
			filesSend++;

		}

	}
	
	return filesSend;
}

void terminateSlaves(int pipesSlave[][2]){
	int i;
	char * terminateMess = "TERMINATE_PROCESS\n";
	for(i=0;i<SLAVE_COUNT;i++){
		write(pipesSlave[i][WRITE_END],terminateMess,strlen(terminateMess));
	}
}



/* 

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
*/