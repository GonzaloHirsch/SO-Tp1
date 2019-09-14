//
// Created by root on 11.09.19.
//

#ifndef QUEUE_CONSTANTS_H
#define QUEUE_CONSTANTS_H


//Definiciones para el view.

#define END_OF_STREAM "End of stream"
#define MAX_NAME_LENGTH 30

#define SHM_NAME_ROOT "sharedBuffer"
#define PUT_GET_SEM_NAME_ROOT "putGetSemNameRoot"
#define MUTEX_NAME_ROOT "mutex"

//Definciones para el application

#define SLAVE_COUNT 5
#define MAX_INFO_TO_SLAVE 300
#define MAX_INFO_FROM_SLAVE 200

#define WRITE_END 1
#define READ_END 0

//Definiciones para el proceso esclavo.

#define FILE_DELIMITER " "
#define FILE_ENDER "\n"

#define TERMINATE_MESSAGE "TERMINATE_PROCESS"
#define ERROR_MESSAGE "ERROR\n"
#define NO_INFO "N/I"

#define MAX_PROCESS_LENGTH 200
#define MAX_INPUT_BUFFER 200
#define MAX_SAT_INPUT 2048
#define MAX_FILES_ALLOWED 100

#define EXIT_STATUS 256

#endif //QUEUE_CONSTANTS_H
