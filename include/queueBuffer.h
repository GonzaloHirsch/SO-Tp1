#ifndef QUEUE_QUEUEBUFFER_H
#define QUEUE_QUEUEBUFFER_H

#include <stdlib.h>

/*
 * Como el struct esta en memoria compartida, y se especifica
 * un tamanio de buffer, para poder desreferenciar memoria
 * correctamente es necesario saber el offset del buffer de
 * caracteres que esta adentro del struct (ver definicion del struct)
 * Para reservar memoria para esta queue, usar el tamanio de buffer
 * deseado mas BUFFER_OFFSET
 */
#define BUFFER_OFFSET (2*sizeof(int) + sizeof(size_t))

typedef struct QueueBufferCDT * QueueBuffer;

//Settea head y tail a 0, y el tamanio del buffer al
//especificado en el parametro correspondiente
void initializeBuffer(QueueBuffer qB, size_t buffSize);
//Checkea si el string entra, inserta el string en el buffer
//empezando en el offset qB->tail. Devuelve 0 en caso de exito,
//-1 si falla.
int putString(QueueBuffer qB, char * string);
//Devuelve un puntero al string en buffer que empieza en el
//offset qB->head (solo lectura) y lo copia a dst, o devuelve
//NULL si el buffer esta vacio
const char * getString(QueueBuffer qB, char * dst);
//Devuelve 1 si el buffer tiene un proximo string para leer, 0 si no
int hasNext(QueueBuffer qB);

#endif //QUEUE_QUEUEBUFFER_H
