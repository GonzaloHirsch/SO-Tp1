#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_PATH_LENGTH 500
#define UNDEFINED -1
#define SAT 0
#define UNSAT 1

typedef struct{
    char fileName[100];
    int numClauses;
    int numVariables;
    char result[10];
    double cpuTime;
    pid_t pidSlave;
}processInfoT;

void analyseSatResults(processInfoT * processInforPointer, char * buffer);


int main(int argc, char * argv[]){

    int  pCount, pid,error;
    char * executeCommandArgs[4] = {"minisat",NULL,NULL};
    //Para leer el resultado
    int pipefd[2]; //Descriptor para el redirigir el output del child.


    char * satEx = "../data/sat1.txt"; 

    //Este ciclo for es unicamente represntativo, despues se va a cambiar
    // por un while(1) con un semaforo adentro.
    for(pCount=1; pCount < 2 ; pCount++){

        char buffer[2048];  //en duda si ponerla aca o afuera.
        pipe(pipefd);//Creamos el canal de comunicacion entre el hijo y el padre
        pid = fork();

        if(-1 == pid){
            perror("Error fork en slaveProcess: ");
            return -1;
        }
        //El hijo que ejecutara el minisat
        if(pid == 0){
            
            //Establecemos el IPC
            close(pipefd[0]);
            dup2(pipefd[1],STDOUT_FILENO);
            close(pipefd[1]);

            //Ejecutamos minisat
            executeCommandArgs[1] = satEx;
            error = execvp(executeCommandArgs[0],executeCommandArgs);
            if(error){
                perror("Failed execvp in slave:");
                return -1;
            }

        }
        else{
            wait(&pCount); //A modificar por waitpid

            //Leemos del hijo.
            close(pipefd[1]); //No necesitamos write end
            while (read(pipefd[0], buffer, sizeof(buffer)) != 0){}

            //TO DO: cambiar obviamente a que esta estructura se almacene en la shared memory.
            processInfoT processInfo;

            //Llenamos la info del process Info
            strcpy(processInfo.fileName, satEx); //TO DO: a cambiar obviamente.
            processInfo.pidSlave = getpid();

            analyseSatResults(&processInfo, buffer);

            printf("%s", buffer);

        }

    }

    
    return 0;
}

void analyseSatResults(processInfoT * processInforPointer, char * buffer){

    char * occurPosition;
    int numberInt = -1;
    double numberDouble = -1;

    occurPosition = strstr(buffer, "Number of variables:");
    if(occurPosition != NULL){
        sscanf(occurPosition, "Number of variables: %d",&numberInt);
        processInforPointer->numVariables = numberInt;
    }
    else{//Caso raro si no esta el dato.
        processInforPointer->numVariables = -1;
        occurPosition = buffer;
    }

    //Notar que para no reccorrer todo de nuevo arrancamos en occurposition del anterior.
    occurPosition = strstr(occurPosition, "Number of clauses:");
    if(occurPosition != NULL){
        sscanf(occurPosition, "Number of clauses: %d",&numberInt);
        processInforPointer->numClauses = numberInt;
    }
    else{ 
        processInforPointer->numClauses = -1;
        occurPosition = buffer;
    }
    
    occurPosition = strstr(occurPosition, "CPU time");
    if(occurPosition != NULL){
        sscanf(occurPosition, "CPU time : %lf ",&numberDouble);
        processInforPointer->cpuTime = numberDouble;
    }
    else{
        processInforPointer->cpuTime = -1;
        occurPosition = buffer;
    }

    //Ahora verificamos si es satisfacible o no.
    occurPosition = strstr(occurPosition,"UNSATISFIABLE");
    if(occurPosition != NULL){
        strcpy(processInforPointer->result,"UNSAT");
    }
    else{
        strcpy(processInforPointer->result,"SAT");
    }
    

}

