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


int main(int argc, char * argv[]){

    int  pCount, pid,error;
    char * executeCommandArgs[4] = {"minisat","-verb=0",NULL,NULL};
    char * results[2]= {"SATISFIABLE","UNSATISFIABLE"};
    int result = -1;

    //Para leer el resultado
    int pipefd[2]; //Descriptor para el redirigir el output del child.


    char * satEx = "../data/sat1.txt"; 

    //Este ciclo for es unicamente represntativo, despues se va a cambiar
    // por un while(1) con un semaforo adentro.
    for(pCount=1; pCount < 2 ; pCount++){

        char buffer[1024];  //en duda si ponerla aca o afuera.
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
            executeCommandArgs[2] = satEx;
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

            //Chequeamos si es satisfacible o no.
            //Vamos leyendo por todo el archivo hasta ver si encontramos si se imprimio
            //que es satisfacible o insatisfacible. Esto es ya que a veces tira warnings y 
            //mensajes extranios a pesar de estar en verbose=0 el programa minisat.
            int i = 0,j = 1, state = UNDEFINED; result = UNDEFINED;
            while(buffer[i] !=0 && result == UNDEFINED){
                switch(state){
                    case UNDEFINED:
                        if(buffer[i] == results[SAT][0]){
                            state = SAT;
                        }
                        else if(buffer[i] == results[UNSAT][0]){
                            state = UNSAT;
                        }
                        break;
                    case SAT:
                        if(results[SAT][j] == 0){
                            result = SAT;
                        }
                        else if(buffer[i] == results[SAT][j]){
                            j++;
                        }
                        else if(buffer[i] == results[UNSAT][0]){ //probablemente innecesario
                            state = UNSAT;
                            j=1;
                        }
                        else{
                            state =UNDEFINED;
                            j=1;
                        }
                        break;
                    case UNSAT:
                        if(results[UNSAT][j] == 0){
                            result = UNSAT;
                        }
                        else if(buffer[i] == results[UNSAT][j]){
                            j++;
                        }
                        else if(buffer[i] == results[SAT][0]){
                            state = SAT;
                            j=1;
                        }
                        else{
                            state =UNDEFINED;
                            j=1;
                        }
                        break;
                }
                i++;
            }

            if(result == SAT){
                printf("Es satisfacible!\n");
            }
            else if(result == UNSAT){
                printf("No es satifacible!\n");
            }
            else{
                printf("Algo salio mal!\n");
            }

        }

    }

    
    return 0;
}
