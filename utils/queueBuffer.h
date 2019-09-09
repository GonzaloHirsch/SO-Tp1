//
// Created by root on 08.09.19.
//

#ifndef SO_TP1_QUEUEBUFFER_H
#define SO_TP1_QUEUEBUFFER_H

#define MAX_BUFF_LENGTH 1024

typedef struct QueueBufferCDT * QueueBuffer;

//sizeof(QueueBufferCDT)
size_t getBufferSize();
//Sets head and tail to 0
void initializeBuffer(QueueBuffer qB);
//Inserts char in buffer; Returns: 0 when successful, -1 when full
int putChar(QueueBuffer qB, char c);
//Returns next char in buffer when successful, 0 when empty
char getChar(QueueBuffer qB);


#endif //SO_TP1_QUEUEBUFFER_H
