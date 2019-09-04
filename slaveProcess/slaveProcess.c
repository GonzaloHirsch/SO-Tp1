#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_PATH_LENGTH 500

int main(int argc, char * argv[]){

    int pCount = argc, i, pid,error;
    char * executeCommand = "minisat";     //Tener cuidado con los archivos con espacios
    char * executeCommandArgs[3] = {"../satExamples/sat2.txt"  ,"../satExamples/satResults.txt",NULL};

    for(i=1; i < 2 ; i++){
        pid = fork();
        if(-1 == pid){
            perror("Error fork en slaveProcess: ");
            return -1;
        }

        if(pid == 0){

            error = execvp(executeCommand,executeCommandArgs);
            if(error){
                perror("Failed execvp in slave:");
                return -1;
            }
        }
        else{
            wait(&pCount);
        }

    }

    
    return 0;
}