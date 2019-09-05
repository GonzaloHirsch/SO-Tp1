#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_PATH_LENGTH 500

int main(int argc, char * argv[]){

    int pCount = argc, i, pid,error;
    char * executeCommandArgs[4] = {"minisat",NULL,"../satExamples/satResults.txt",NULL};
    
    //Para leer del archivo donde se guarda el resultado.
    char result[100];
    FILE * resultPointer;

    if((resultPointer = fopen("../satExamples/satResults.txt", "r")) == -1){
        perror("Error in opening file in slave: ");
        return -1;
    }


    char * satEx = "../satExamples/sat1.txt"; 

    for(i=1; i < 2 ; i++){
        pid = fork();
        if(-1 == pid){
            perror("Error fork en slaveProcess: ");
            return -1;
        }

        if(pid == 0){

            executeCommandArgs[1] = satEx;
            error = execvp(executeCommandArgs[0],executeCommandArgs);
            if(error){
                perror("Failed execvp in slave:");
                return -1;
            }

        }
        else{
            wait(&pCount); //A modifiacr por waitpid
            fscanf(resultPointer, "%[^\n]", result);

            //Si es satfisfacible
            if(strncmp("SAT", result,3) == 0){
                //Ejecutamos acciones para satisfacible.
                printf("Es satisfacible!\n");
            }
            //Caso que no es satisfacible...
            else if(strncmp("UNSAT", result,5) == 0){
                //Ejecutamos codigo de que no es satisfacible
                printf("NO es satisfacible!\n");
            }
            else{
                //Codigo de error
                printf("Codigo extranio!\n");
            }
        


        }

    }

    
    return 0;
}
