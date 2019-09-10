#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAX_PROCESS_LENGTH 200
#define MAX_INPUT_BUFFER 200
#define MAX_SAT_INPUT 2048

#define TERMINATE_MESSAGE "TERMINATE_PROCESS"

#define INITIAL_PROCESSES 2

int processSat(char * inputBuffer);
void analyseSatResults(char * processInfo, char * buffer, char * fileName);
void readFromStdin(char * inputBuffer, int size);
void cleanBuffer(char * buffer, int size);


int main(int argc, char * argv[]){

    char inputBuffer[MAX_INPUT_BUFFER];



    readFromStdin(inputBuffer, MAX_INPUT_BUFFER);
    while(strncmp(inputBuffer,TERMINATE_MESSAGE,sizeof(TERMINATE_MESSAGE) != 0)){

        //Creamos el esclavo y procesamos el sat.
        //TO DO: ver que pasa si fallamos a procesar el minisat.
        processSat(inputBuffer);
    
        //Limpiamos el buffer y leemos el input que nos manden.
        cleanBuffer(inputBuffer,MAX_INPUT_BUFFER);
        readFromStdin(inputBuffer,MAX_INPUT_BUFFER);        

    }



    //TESTING
    printf("Slave Terminated\n");

    
    return 0;
}
/*
Creates a slave, procceses and parsed the info of the sat.
Sends the info via stdout to application process.
 */
int processSat(char * inputBuffer){

    int  pid,error,pCount;
    char * executeCommandArgs[3] = {"minisat",NULL,NULL};
    int pipefd[2]; //Descriptor para el redirigir el output del child.
    char satBuffer[MAX_SAT_INPUT]; 

    
    pipe(pipefd);//Creamos el canal de comunicacion entre el hijo y el padre
    pid = fork();

    if(-1 == pid){
        perror("Error fork en slaveProcess: ");
        return -1;
    }
    //El hijo que ejecutara el minisat
    if(pid == 0){
        
        //Establecemos el IPC con el hijo que va a ejecutar el minisat.
        close(pipefd[0]);
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[1]);

        //Ejecutamos minisat
        executeCommandArgs[1] = inputBuffer;
        error = execvp(executeCommandArgs[0],executeCommandArgs);
        if(error){
            perror("Failed executing minisat in slave:");
            return -1;
        }

    }
    else{ //TODO: fijarse como hacer por si falla el execvp del slave termine.

        wait(&pCount); //A modificar por waitpid?

        //Leemos del hijo.
        close(pipefd[1]); //No necesitamos write end
        while (read(pipefd[0], satBuffer, sizeof(satBuffer)) != 0){}

        //Guardamos la informacion del proceso en processInfo
        char processInfo[MAX_PROCESS_LENGTH];
        analyseSatResults(processInfo, satBuffer,inputBuffer);

        //Imprimimos en stdout
        printf(processInfo);

    }

    return 0;

}

void analyseSatResults(char * processInfo, char * buffer, char * fileName){

    char * occurPosition;
    char numberOfVariables[5];
    char numberOfClauses[5];
    char cpuTime[15];
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
    else if(strstr(buffer, "SATISFIABLE")!= NULL){ 
        satisfacible = "SAT";
    }
    else{
        perror("Error finding satisfacibility.");
    }
    
    sprintf(processInfo,"%s\n%s\n%s\n%s\n%s\n%d\n", fileName, numberOfClauses,numberOfVariables,
    satisfacible,cpuTime,getpid());


}



void readFromStdin(char * inputBuffer, int size){
    //Leemos de la stdin hasta que ingrese una newline.
    char c = 0;
    int cnt = 0;
    while(read(STDIN_FILENO, &c, 1) != 0 && cnt < size - 1){
        if(c == '\n'){
            inputBuffer[cnt] = 0;
            return;
        }
        inputBuffer[cnt++] = c;
    }
    inputBuffer[cnt] = 0; //Por las dudas, hay que asegurar que termine con 0.
    return;
}

void cleanBuffer(char * buffer, int size){
    int i=0;
    for(;i<size;i++){
        buffer[i] = 0;
    }
}
