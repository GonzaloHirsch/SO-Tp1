#include "../include/buffer.h"
#include <string.h>

typedef struct BufferCDT{

    char buff[MAX_BUFF_LENGTH];

}BufferCDT;

size_t getBufferSize(){
    return sizeof(BufferCDT);
}

int putString(Buffer qB, char * string){

    if(strcpy(qB->buff, string)) return 0;
    return -1;

}

char * getString(Buffer qB){

    return qB->buff;
}
