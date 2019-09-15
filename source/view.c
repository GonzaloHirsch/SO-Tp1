// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

    //esto es un buffer para crear y guardar los nombres de semaforos
    char namesBuffer[MAX_NAME_LENGTH];
    sprintf(namesBuffer, "%s%d", SHM_NAME_ROOT, pid);

    //abriendo la shm (file descriptor, truncado, mapeo)
    int sharedBufferFd = shm_open(namesBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, size + BUFFER_OFFSET);
    QueueBuffer qB = (QueueBuffer) mmap(0, size + BUFFER_OFFSET, PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);

    //1 semaforo indicando que hay contenido para leer
    //1 mutex para coordinar lectura y escritura (ie accesos) a la shm
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


    //cierre de fds, unmapping de shm y desvinculacion de semaforos

    munmap(qB, size + BUFFER_OFFSET);
    close(sharedBufferFd);

    sem_close(putGetSem);
    sem_close(mutex);

    sprintf(namesBuffer, "%s%d", PUT_GET_SEM_NAME_ROOT, pid);
    sem_unlink(namesBuffer);
    sprintf(namesBuffer, "%s%d", MUTEX_NAME_ROOT, pid);
    sem_unlink(namesBuffer);

    return 0;
}

void printData(const char *buff) {

    static char auxBuff[MAX_INFO_FROM_SLAVE];

    //esto es para que lo que esta en buff no se modifique
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
