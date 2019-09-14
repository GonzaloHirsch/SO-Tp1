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
//Semaphore includes
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
//Definitions includes
#include "../include/constants.h"
#include "../include/queueBuffer.h"



int createSlaves(int count,int pipesSlave[][2]);
void readInfoSlave(int pipesSlave[][2], int slaveNum, char *tempBuffer);
void sendInfoSlave(int pipesSlave[][2],int slaveNum, char ** filesToProcess, int filesSend);
int sendInitialFiles(int filesCant, int pipesSlave[][2], char ** filesToProcess);
void terminateSlaves(int pipesSlave[][2]);
void sendInfoToView(char *buffer, QueueBuffer qB, sem_t *putGetSem, sem_t *mutex);
void saveInfoResult(FILE * resultFile, char * buffer);

void terminateView();

int main(int argc, char * argv[]){

    //Esperamos al vista por 2 segundos.
    sleep(2);

	//------DEFINICIONES MANEJO DE ESCLAVOS Y ARCHIVO RESULT---------------------------------
	int filesSend = 0,filesCant = argc-1,filesRec =0;
	int pipesSlave[SLAVE_COUNT][2];
	//Los archivos a procesar empiezan con el 2do archivo de los argumentos.
	char ** filesToProcess = argv+1;

	FILE * resultFile = fopen("result.txt","w");

	//----------------------------------------------------------------------

    //todo integration with view / beginning of view ipc preparation code

	//-------MANEJO DE SHM Y VISTA-----------------------------------------------------------

    //Example user-defined buffer size
    size_t size = argc*MAX_INFO_FROM_SLAVE;
    int pid = getpid();

    //Print to stdout for piping with view
    fprintf(stdout,  "%d\n%lu\n", pid, (unsigned long) size);

    char namesBuffer[MAX_NAME_LENGTH];
    sprintf(namesBuffer, "%s%d", SHM_NAME_ROOT, pid);

    //Opening the shm (file descriptor, truncating, mapping)
    int sharedBufferFd = shm_open(namesBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, size + BUFFER_OFFSET);
    QueueBuffer qB = (QueueBuffer) mmap(0, size + BUFFER_OFFSET, PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);
    initializeBuffer(qB, size);

    //1 semaphore for indicating there is content to read
    //1 mutex semaphore for performing operations on memory
    sprintf(namesBuffer, "%s%d", PUT_GET_SEM_NAME_ROOT, pid);
    sem_t * putGetSem = sem_open(namesBuffer, O_CREAT, 0600, 0);
    sprintf(namesBuffer, "%s%d", MUTEX_NAME_ROOT, pid);
    sem_t * mutex = sem_open(namesBuffer, O_CREAT, 0600, 1);

    //todo tengo que ver esto
    sem_post(mutex);

    //todo end of view ipc preparation code

	//-------------------------------------------------------------------------------------

	//----------------MANEJO DE MULTIPIPES-------------------------------------------
	//Creamos el set de pipes a esperar para que lean.
	fd_set pipeReadSet;
	int i=0, pipesChecked = 0 ,numPipesReady;
	//Struct que indica el tiempo a max a esperar a recibir data.
	struct timeval tv;
	tv.tv_sec = 60;
    tv.tv_usec = 0;

	//-------------------------------------------------------------------------------


    //Creamos los esclavos
	createSlaves(SLAVE_COUNT,pipesSlave);

	//Distribuimos los archivos iniciales
	filesSend = sendInitialFiles(filesCant,pipesSlave,filesToProcess);

	//Inicializamos el pipeReadSet para el select.
	FD_ZERO(&pipeReadSet);
	for(i=0;i<SLAVE_COUNT;i++){
		FD_SET(pipesSlave[i][READ_END],&pipeReadSet);
	}


	//Recibimos resultados de los esclavos y enviamos archivos a los esclavos

	while(filesRec < filesCant){

		//Esperaremos hasta que por lo menos un pipe tenga informacion disponible para leer.
		//Si en 1 minuto no recibimos nada, termina.
		numPipesReady = select(FD_SETSIZE, &pipeReadSet, NULL,NULL, &tv);

		if(numPipesReady == -1){
			perror("Error in select():");
			break;
		}
		else if(numPipesReady == 0){
			perror("No data received in 60 seconds");
			break;
		}

		pipesChecked = 0; //Contamos cuantos archivos vamos procesando en este select.

        //Buffer para guardar la informacion recibida por el esclavo.
        char tempBuffer[MAX_INFO_FROM_SLAVE];

		//Buscamos en cada pipe de los esclavos cual de ellos recibio info.
		//Si ya llegamos a la cantidad de numPipesReady entonces no hay que leer mas.
		for(i=0;i<SLAVE_COUNT && pipesChecked < numPipesReady;i++){

			//Si ese pipe recibio info...
			if(FD_ISSET(pipesSlave[i][READ_END],&pipeReadSet)){
				
				//Leemos la informacion recibida del esclavo.
                readInfoSlave(pipesSlave, i, tempBuffer);
				//Mandamos la informacion a el proceso vista.
                sendInfoToView(tempBuffer, qB, putGetSem, mutex);
				//Guardamos la info en un archivo resultado.
				saveInfoResult(resultFile,tempBuffer);

				filesRec++;

				//Si todavia quedan archivos por mandar le enviamos 1 mas.
				if(filesSend < filesCant){
					sendInfoSlave(pipesSlave,i,filesToProcess,filesSend);
					filesSend++;
				}

				pipesChecked++;
			}		
		}

		//Re seteamos el set.
		FD_ZERO(&pipeReadSet);
		for(i=0;i<SLAVE_COUNT;i++){
			FD_SET(pipesSlave[i][READ_END],&pipeReadSet);
		}
		
		
	}

	//Mandamos la signal a view que no hay mas archivos a procesar.
	sendInfoToView(END_OF_STREAM, qB, putGetSem, mutex);
	//Terminamos a los archivos esclavos y cerramos sus pipes.
	terminateSlaves(pipesSlave);
	//Cerramos el archivo result.
	fclose(resultFile);
    //Terminacion de shared memory.
    close(sharedBufferFd);
    munmap(qB, STD_BUFF_LENGTH + BUFFER_OFFSET);
	//Cierre de semaforos
    sem_close(putGetSem);
    sem_close(mutex);

	return 0;
}



void sendInfoToView(char *buffer, QueueBuffer qB, sem_t *putGetSem, sem_t *mutex) {
    sem_wait(mutex);
    //todo write whatever to view
    putString(qB, buffer);
    sem_post(mutex);
    sem_post(putGetSem);
}

/*
	Crea SLAVE_COUNT esclavos y un pipe bidireccional correspondiente a cada uno.
	Guarda la informacion de los pipes correspondientes en pipesSlave.
 */

int createSlaves(int count,int pipesSlave[][2]){
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

		} 	
	}

	return 0;
}

/*
	Lee la informacion del archivo esclavo hasta encontrar el \n y lo guarda en tempBuffer.
 */

void readInfoSlave(int pipesSlave[][2], int slaveNum, char *tempBuffer) {

    char c = 0;
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

}
/*
	Envia un nuevo archivo al proceso esclavo.
 */

void sendInfoSlave(int pipesSlave[][2],int slaveNum, char ** filesToProcess, int filesSend){
	write(pipesSlave[slaveNum][WRITE_END], filesToProcess[filesSend],strlen(filesToProcess[filesSend]));
	write(pipesSlave[slaveNum][WRITE_END],"\n",1);
}

/*
	Funcion para repartir los archivos iniciales.
	Reparte entre 1 y MAX_FILES_ALLOWED a todos los esclavos disponibles. Toma la mitad
	de los archivos a procesar y los divide por la cantidad de esclavos disponibles.
	Devuelve la cantidad de archivos enviados.
 */
int sendInitialFiles(int filesCant, int pipesSlave[][2], char ** filesToProcess){
	//Take half of the files and distribute an equal quantity to each slave.

	int filesPerSlave = (filesCant/2)/SLAVE_COUNT;
	int filesSend = 0, slaveNum ,i;

	//Caso especial: Si inicialmente tenemos mas esclavos que la mitad de archivos a procesar
	//le repartimos 1 a cada uno. 
	if(filesPerSlave == 0)
		filesPerSlave = 1;
	//Si superamos la cantidad de archivos maxima por esclavo, le asignamos simplemente la cant maxima
	else if(filesPerSlave > MAX_FILES_ALLOWED)
		filesPerSlave = MAX_FILES_ALLOWED;
	
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

/*
	Manda la signal de terminacion a todos los procesos.
	Cierra los pipes que estaban abiertos
 */
void terminateSlaves(int pipesSlave[][2]){
	int i;
	char * terminateMess = TERMINATE_MESSAGE;
	for(i=0;i<SLAVE_COUNT;i++){
		write(pipesSlave[i][WRITE_END],terminateMess,strlen(terminateMess));
		close(pipesSlave[i][WRITE_END]);
		close(pipesSlave[i][READ_END]);
	}

}
/*
	Guardamos la informacion de los resultados en el archivo results.txt.
 */

void saveInfoResult(FILE * resultFile, char * buffer){
	static char auxBuff[MAX_INFO_FROM_SLAVE];
    //so that bufer doesn't get modified
    strcpy(auxBuff, buffer);

    char * info[6] = {"Nombre del Archivo", "Cantidad de Clausulas", "Cantidad de Variables", "Resultado", "Tiempo de Procesamiento", "ID del Esclavo"};
    char * token = strtok(auxBuff, FILE_DELIMITER);

    int i;
    for(i=0; i<6; i++){
		fprintf(resultFile, "%s: %s\n", info[i],token);
        token = strtok(NULL, FILE_DELIMITER);
    }
	fprintf(resultFile, "\n");
    
}