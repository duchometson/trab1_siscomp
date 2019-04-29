/*  Fiquei com algumas dúvidas a respeito do programa e parei aqui para a gente decidir:
 *  1) Leitura dos comandos para executar os programas e começar a preencher as filas do escalonador será feita como? Inserido na linha de comando? Lendo arquivos externos, acho válido perguntar para o Seibel
 * ex: exec prog1 (1,2,3) tem que ser lido de onde
 *  
 *  2) O método Round Robin segue essa lógica aqui:https://pt.wikipedia.org/wiki/Round-robin
 *     Parece ser bem simples, so fiquei com dúvida que de como a gente vai fazer para retomar um processo parado. O enunciado fala que usaremos sigstop e sigcont para pausar e retomar o processo, sinal ja recomeça o processo a partir do ponto que ele parou?  . Será 
 * 
 */



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>

int LIMITE_FILA_PROGRAMAS = 10;
int NUM_FILAS = 3;

struct programa {
    int pid_processo;
    int prioridade;
    char nome[10];
    int estaRodando;
    
}; typedef struct programa Programa;

void pausaProcesso();
void continuaProcesso();
void eliminaProcesso();
int escalonador(Programa *F1, Programa *F2, Programa *F3);


int main(int argc, char *argv[]) {

    Programa *F1;
    Programa *F2;
    Programa *F3;
    int *tamFilas;
    
    int segmentoFila1, segmentoFila2, segmentoFila3;
    int segmentoFilasTam;
    
    ///// Criação de Espaço de memória para filas do escalonador///////
    segmentoFila1 = shmget (IPC_PRIVATE, sizeof (Programa) * limiteProgramas, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F1 = (Programa*)shmat(segmentoFila1,0,0);
     
    segmentoFila2 = shmget (IPC_PRIVATE, sizeof (Programa) * limiteProgramas, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F2 = (Programa*)shmat(segmentoFila2,0,0);
     
    segmentoFila3 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_FILA_PROGRAMAS, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F3 = (Programa*)shmat(segmentoFila3,0,0);
    
    
    signal(SIGCHD, eliminaProcesso); // cadastra o catcher de quando um processo é finalizado
    
    
    /* Criação de Espaço de memórias para vetor com o tamanho de cada fila 
     * tamFilas[0] = tamanho de F1;
     * tamFilas[1] = tamanho de F2;                                                                    
     * tamFilas[2] = tamanho de F3;                                                                    */
    segmentoFilasTam = shmget (IPC_PRIVATE, sizeof (int) * NUM_FILAS , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    tamFilas = (int*)shmat(segmentoFilasTam,0,0);

    pid = fork();
     
    if( pid == 0 ) {                         // Filho
        //desenvolver leitor para preencher as filas
    } else {                                 // Pai
        escalonador(F1, F2, F3, tamFilas);         
    }
     
    //signal(SIGSTOP, pausaProcesso);  sigstop não pode ser capturado
    //signal(SIGCONT, continuaProcesso); como sigstop não pode ser capturado não precisaremos capturar esse sinal tb.
    
    shmdt(F1);
    shmdt(F2);
    shmdt(F3);
    shmdt(tamFilas);
    
}

int escalonador( Programa *F1, Programa *F2, Programa *F3, int *tamFilas) {
    int i = 0;
    int processosFinalizados = 0;
    int processosExistentes = tamFilas[0] + tamFilas[1] + tamFilas[2];
    while( processosFinalizados < numeroExistentes ) {
    // temos que aplicar a lógica round robin - https://pt.wikipedia.org/wiki/Round-robin
    }
}

void processoFinalizado(int sig) {
    printf("Processo %d terminado", sig);
}
