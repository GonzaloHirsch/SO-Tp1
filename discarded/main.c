#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "./include/queueBuffer.h"
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include "include/constants.h"

static char * shmNameRoot = "/sharedBuffer";
static char * putGetSemNameRoot = "/putGetSem";
static char * mutexNameRoot = "/mutex";

int main(int argc, char * argv[]) {

    //Example user-defined buffer size
    size_t size = STD_BUFF_LENGTH;
    int pid = getpid();

    //Print to stdout for piping with view
    fprintf(stdout,  "%d\n%d\n", getpid(), size);

    char namesBuffer[MAX_NAME_LENGTH];
    sprintf(namesBuffer, "%s%d", shmNameRoot, pid);

    //Opening the shm (file descriptor, truncating, mapping)
    int sharedBufferFd = shm_open(namesBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, size + BUFFER_OFFSET);
    QueueBuffer qB = (QueueBuffer) mmap(0, size + BUFFER_OFFSET, PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);

    //1 semaphore for indicating there is content to read
    //1 mutex semaphore for performing operations on memory
    sprintf(namesBuffer, "%s%d", putGetSemNameRoot, pid);
    sem_t * putGetSem = sem_open(namesBuffer, O_CREAT, 0600, 0);
    sprintf(namesBuffer, "%s%d", mutexNameRoot, pid);
    sem_t * mutex = sem_open(namesBuffer, O_CREAT, 0600, 1);

    char auxBuff[1024];

    //sem_post(mutex);


    for(int i = 0; i<10; i++){
        sem_wait(mutex);
            //todo write whatever to view
            sprintf(auxBuff, "String %d\n", i);
            putString(qB, auxBuff);
        sem_post(mutex);
        sem_post(putGetSem);
    }

    //signal view to end without actually sending a signal
    putString(qB, END_OF_STREAM);

    //housekeeping
    munmap(qB, STD_BUFF_LENGTH + BUFFER_OFFSET);
    close(sharedBufferFd);

    sem_close(putGetSem);
    sem_close(mutex);

    return 0;
}