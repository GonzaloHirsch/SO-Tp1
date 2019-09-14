//This is a personal academic project. Dear PVS-Studio, please check it.
//PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/queueBuffer.h"
#include <stdio.h>
#include <string.h>


typedef struct QueueBufferCDT{

    size_t size;
    int head, tail;
    char buff[];

}QueueBufferCDT;

//copies the full string including 0 char at the end
static int strcopy(char * dst, const char * src){

    int aux=0;
    while(*(src+aux)){
        *(dst+aux)=*(src+aux);
        aux++;
    }
    *(dst+aux)=0;
    aux++;
    return aux;
}


void initializeBuffer(QueueBuffer qB, size_t buffSize){
    qB->head = qB->tail = 0;
    qB->size = buffSize;
}


int putString(QueueBuffer qB, char * string){

    //check if string fits
    if(qB->tail + strlen(string) < qB->size) {
        qB->tail += strcopy(&qB->buff[qB->tail], string);
        return 0;
    }
    return -1;
}

char * getString(QueueBuffer qB, char * dst){

    if(!hasNext(qB)) return NULL;
    char * aux = &qB->buff[qB->head];
    //avanzo hasta el head proximo string
    qB->head += strcopy(dst, &qB->buff[qB->head]);
    return aux;
}

int hasNext(QueueBuffer qB){
    return qB->head < qB->tail;
}