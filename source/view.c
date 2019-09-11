#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/queueBuffer.h"
#include "../include/constants.h"


static char * shmNameRoot = "/sharedBuffer";
static char * putGetSemNameRoot = "/putGetSem";
static char * mutexNameRoot = "/mutex";



int main(int argc, char * argv){

    char pid;
    size_t size;

    fscanf(stdin, "%d\n", &pid);
    fscanf(stdin, "%ld\n", &size);

    //todo remove test line
    printf("app pid: %d\nbuff_size:%ld\n\n", pid, size);


    //this is a buffer for shm/semaphore names
    char namesBuffer[MAX_NAME_LENGTH];
    sprintf(namesBuffer, "%s%d", shmNameRoot, pid);

    //opening the shm (file descriptor, truncating, mapping)
    int sharedBufferFd = shm_open(namesBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, size + BUFFER_OFFSET);
    QueueBuffer qB = (QueueBuffer) mmap(0, size + BUFFER_OFFSET, PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);

    //1 semaphore for indicating there is content to read
    //1 mutex semaphore for performing operations on memory
    sprintf(namesBuffer, "%s%d", putGetSemNameRoot, pid);
    sem_t * putGetSem = sem_open(namesBuffer, O_CREAT);
    sprintf(namesBuffer, "%s%d", mutexNameRoot, pid);
    sem_t * mutex = sem_open(namesBuffer, O_CREAT);


    char readBuff[1024];

    printf("%1024s\n", readBuff);
    while(strcmp(readBuff, END_OF_STREAM) != 0){
        printf("Waiting for putGetSem...\n");
        sem_wait(putGetSem);
        printf("got putGetSem, waiting for mutex\n");
        sem_wait(mutex);
        if(hasNext(qB))
            printf("%s\n", getString(qB, readBuff));
        printf("%s\n", getCurrentString(qB));
        printf("posting on mutex\n");
        sem_post(mutex);
    }

    free(namesBuffer);
}