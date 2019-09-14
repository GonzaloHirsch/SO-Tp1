#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/queueBuffer.h"
#include "../include/constants.h"


void printData(const char *buff);

int main(int argc, char * argv[]){

    int pid;
    size_t size;

    char * end;
    char read[MAX_STDIN_INPUT];

    fgets(read, MAX_STDIN_INPUT, stdin);
    pid = (int) strtol(read, &end, 10);
    fgets(read, MAX_STDIN_INPUT, stdin);
    size = strtol(read, &end, 10);

    //this is a buffer for shm/semaphore names
    char namesBuffer[MAX_NAME_LENGTH];
    sprintf(namesBuffer, "%s%d", SHM_NAME_ROOT, pid);

    //opening the shm (file descriptor, truncating, mapping)
    int sharedBufferFd = shm_open(namesBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, size + BUFFER_OFFSET);
    QueueBuffer qB = (QueueBuffer) mmap(0, size + BUFFER_OFFSET, PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);

    //1 semaphore for indicating there is content to read
    //1 mutex semaphore for performing operations on memory
    sprintf(namesBuffer, "%s%d", PUT_GET_SEM_NAME_ROOT, pid);
    sem_t * putGetSem = sem_open(namesBuffer, O_CREAT);
    sprintf(namesBuffer, "%s%d", MUTEX_NAME_ROOT, pid);
    sem_t * mutex = sem_open(namesBuffer, O_CREAT);


    char readBuff[MAX_INFO_FROM_SLAVE]={0};

    while(1){
        sem_wait(putGetSem);
        sem_wait(mutex);
        if(hasNext(qB))
            getString(qB, readBuff);
        sem_post(mutex);
        if(strcmp(readBuff, END_OF_STREAM) == 0)
            break;
        printData(readBuff);
    }


    munmap(qB, STD_BUFF_LENGTH + BUFFER_OFFSET);
    close(sharedBufferFd);

    sem_close(putGetSem);
    sem_close(mutex);

    return 0;
}

void printData(const char *buff) {

    static char auxBuff[MAX_INFO_FROM_SLAVE];

    //so that buff doesn't get modified
    strcpy(auxBuff, buff);

    char * info[6] = {"Nombre del Archivo", "Cantidad de Clausulas", "Cantidad de Variables", "Resultado", "Tiempo de Procesamiento", "ID del Esclavo"};

    char * token = strtok(auxBuff, FILE_DELIMITER);

    int i;
    for(i= 0; i<6; i++){
        printf("%s: %s\n", info[i], token);
        token = strtok(NULL, FILE_DELIMITER);
    }
    printf("\n");
}
