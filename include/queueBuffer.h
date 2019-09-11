#ifndef QUEUE_QUEUEBUFFER_H
#define QUEUE_QUEUEBUFFER_H

#include <stdlib.h>

#define MAX_BUFF_LENGTH 1024
#define STD_BUFF_LENGTH 1024
//Since struct is in shared memory, and a buffer size
//is specified, in order to dereference memory correctly
//it is useful (necessary) to know the offset of the char buff inside the struct
//When reserving memory for this queue, set the size to the desired buffer size
//plus BUFFER_OFFSET
#define BUFFER_OFFSET (2*sizeof(int) + sizeof(size_t))

typedef struct QueueBufferCDT * QueueBuffer;

//Sets head and tail to 0
void initializeBuffer(QueueBuffer qB, size_t buffSize);
//Checks if string fits, insert string in buffer starting
//at offset qB->tail. Returns 0 if successful, -1 if not.
int putString(QueueBuffer qB, char * string);
//Returns pointer to string in buffer starting at offset
//qB->head and copies it to dst, or only returns NULL if
//buffer is empty
char * getString(QueueBuffer qB, char * dst);
//Returns 1 if buffer has a next string to read,
//0 if not
int hasNext(QueueBuffer qB);
//Returns current string in buffer starting at
//offset qB->head without proceeding to the next string
char * getCurrentString(QueueBuffer qB);
//Inserts char in buffer; Returns: 0 when successful, -1 when full todo delete:unused
int putChar(QueueBuffer qB, char c);
//Returns next char in buffer when successful, 0 when empty todo delete:unused
char getChar(QueueBuffer qB);
//Writes a string to buffer

#endif //QUEUE_QUEUEBUFFER_H
