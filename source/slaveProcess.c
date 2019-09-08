#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_PROCESS_LENGTH 200
#define UNDEFINED -1
#define SAT 0
#define UNSAT 1

void analyseSatResults(char * processInfo, char * buffer, char * fileName);


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

            //Guardamos la informacion del proceso en processInfo
            char processInfo[MAX_PROCESS_LENGTH];
            analyseSatResults(processInfo, buffer,satEx);


        }

    }

    
    return 0;
}

void analyseSatResults(char * processInfo, char * buffer, char * fileName){

    char * occurPosition;
    char numberOfVariables[5];
    char numberOfClauses[5];
    char cpuTime[10];
    char * satisfacible;

    occurPosition = strstr(buffer, "Number of variables:");
    if(occurPosition != NULL){
        sscanf(occurPosition, "Number of variables: %s", numberOfVariables);
    }
    else{//Caso raro si no esta el dato.
        perror("Failure finding the number of variables");
    }

    //Notar que para no reccorrer todo de nuevo arrancamos en occurposition del anterior.
    occurPosition = strstr(occurPosition, "Number of clauses:");
    if(occurPosition != NULL){
        sscanf(occurPosition, "Number of clauses: %s",numberOfClauses);
    }
    else{ 
        perror("Failure finding the number of clauses");
    }
    
    occurPosition = strstr(occurPosition, "CPU time");
    if(occurPosition != NULL){
        sscanf(occurPosition, "CPU time : %s ",cpuTime);
    }
    else{
        perror("Failure finding CPU time");
    }

    //Ahora verificamos si es satisfacible o no.
    occurPosition = strstr(occurPosition,"UNSATISFIABLE");
    if(occurPosition != NULL){
        satisfacible = "UNSAT";
    }
    else{
        satisfacible = "SAT";
    }
    
    sprintf(processInfo,"%s\n%s\n%s\n%s\n%s\n%d\n", fileName, numberOfClauses,numberOfVariables,
    satisfacible,cpuTime,getpid());

    printf(processInfo);

}

