//
// Created by root on 08.09.19.
//

#include "queueBuffer.h"

typedef struct QueueBufferCDT{

    char buff[MAX_BUFF_LENGTH];
    int head, tail;

}QueueBufferCDT;

size_t getBufferSize(){
    return sizeof(QueueBufferCDT);
}

void initializeBuffer(QueueBuffer qB){
    qB->head = qB->head = 0;
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

