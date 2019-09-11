#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/buffer.h"

static char * shmNameRoot = "/sharedBuffer";
static char * useSemNameRoot = "/useSem";
static char * rwSemNameRoot = "/rwSem";
const char * terminationCode = "NO_MORE_RESULTS";

int main(int argc, char * argv){

    char pid;
    size_t size;

    fscanf(stdin, "%d\n", &pid);
    fscanf(stdin, "%ld\n", &size);

    //todo remove test line
    printf("app pid: %d\nbuff_size:%ld\n\n", pid, size);


    //auxBuffer is a dynamically allocated string for temporarily storing the names of
    //semaphore/shm names
    char * auxBuffer = malloc(strlen(shmNameRoot) + strlen(argv[1]));
    sprintf(auxBuffer, "%s%d", shmNameRoot, argv[1]);

    int sharedBufferFd = shm_open(auxBuffer, O_CREAT | O_RDWR, 0600);
    ftruncate(sharedBufferFd, getBufferSize());
    Buffer theBuffer = (Buffer) mmap(0, getBufferSize(), PROT_WRITE | PROT_READ, MAP_SHARED, sharedBufferFd, 0);

    //Code related to useSem (flag indicating the process is reading/writing to shm)
    sprintf(auxBuffer, "%s%s", useSemNameRoot, argv[1]);
    sem_t * useSem = sem_open(auxBuffer, O_CREAT | O_RDWR, 0600);
    sprintf(auxBuffer, "%s%s", rwSemNameRoot);
    sem_t * rwSem = sem_open(auxBuffer, O_CREAT | O_RDWR, 0600);
    //Freeing auxBuffer - no longer used
    free(auxBuffer);



    while(1) {
        sem_wait(useSem);
        sem_wait(rwSem);

        getString(theBuffer);

        sem_post(rwSem);
    }

    free(auxBuffer);
}