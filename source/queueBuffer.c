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

const char * getCurrentString(QueueBuffer qB){
    return &qB->buff[qB->head];
}

int hasNext(QueueBuffer qB){
    return qB->head < qB->tail;
}

int putChar(QueueBuffer qB, char c){

    //Returns -1 when full
    if((qB->tail + 1) % MAX_BUFF_LENGTH == qB->head){
        return -1;
    }

    qB->buff[qB->tail++] = c;

    return 0;
}

char getChar(QueueBuffer qB){

    //Returns 0 when empty
    if(qB->head == qB->tail){
        return 0;
    }

    char aux = qB->buff[qB->head++];
    return aux;
}