#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/buffer.h"

static char * nameRoot = "/sharedBuffer";
static char * useSemNameRoot = "/useSem";
static char * rwSemNameRoot = "/rwSem";
const char * terminationCode = "NO_MORE_RESULTS";

int main(int argc, char * argv){

    if(argc != 2){
        return -1;
    }

    //auxBuffer is a dynamically allocated string for temporarily storing the names of
    //semaphore/shm names
    char * auxBuffer = malloc(strlen(nameRoot) + strlen(argv[1]));
    sprintf(auxBuffer, "%s%s", nameRoot, argv[1]);

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

        getString(qB);

        sem_post(rwSem);
    }

    free(auxBuffer);
}