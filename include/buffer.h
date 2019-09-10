#ifndef QUEUE_QUEUEBUFFER_H
#define QUEUE_QUEUEBUFFER_H

#include <stdlib.h>

#define MAX_BUFF_LENGTH 1024

typedef struct BufferCDT * Buffer;

//sizeof(BufferCDT)
size_t getBufferSize();
//Sets head and tail to 0
void initializeBuffer(Buffer qB);
//Writes string to buffer
int putString(Buffer qB, char * string);
//Reads string from buffer
char * getString(Buffer qB);

#endif //QUEUE_QUEUEBUFFER_H
