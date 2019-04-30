/********************************************************************
 *  Fiquei com algumas dúvidas a respeito do programa e parei aqui para a gente decidir:
 *  1) Leitura dos comandos para executar os programas e começar a preencher as filas do escalonador será feita como? Inserido na linha de comando? Lendo arquivos 
 *     externos, acho válido perguntar para o Seibel. Ou seja como iremos preencher as filas F1, F2 e F3. Ele chama de "interpretador" no enunciado
 * ex: exec prog1 (1,2,3) tem que ser lido de onde
 *  
 *  2) O método Round Robin segue essa lógica aqui:https://pt.wikipedia.org/wiki/Round-robin
 *     Parece ser bem simples, so fiquei com dúvida que de como a gente vai fazer para retomar um processo parado. O enunciado fala que usaremos sigstop e sigcont para 
 *     pausar e retomar o processo, sinal ja recomeça o processo a partir do ponto que ele parou?
 * 
 *  3) Precisamos saber como iremos simular o comportamento I/O Bound. O enunciado pede para que a gente suba a prioridade de um processo no momento que esse 
 *     comportamennto for apresentado, mas não sei como podemos fazer.
 * 
 ********************************************************************/
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
#define EVER ;;


int LIMITE_PROGRAMAS_FILA = 10;
int NUM_FILAS = 3;

struct programa {
    int pid_processo;
    int prioridade;
    char nome[10];
    short int esta_rodando;
    
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
    
    int status;
    int segmentoFila1, segmentoFila2, segmentoFila3;
    int segmentoFilasTam;
    
    ///// Criação de Espaço de memória para filas do escalonador///////
    segmentoFila1 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F1 = (Programa*)shmat(segmentoFila1,0,0);
     
    segmentoFila2 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F2 = (Programa*)shmat(segmentoFila2,0,0);
     
    segmentoFila3 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
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
        waitpid(pid, &status, NULL)
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
    int aux = 0, i = 0, pid, status;
    int quantum = 3;
    int processosFinalizados = 0;
    Programa filas[3];
    int processosExistentes = tamFilas[0] + tamFilas[1] + tamFilas[2];
    
    filas[0] = F1;
    filas[1] = F2;
    filas[2] = F3;
    
    while( processosFinalizados < processosExistentes ) {
        while( aux < 3 ) { // tratamento para as filas em geral
            while( tamFilas[aux] >= i ) { // tratamento para a filas individualmente
                if( processosFinalizados < tamFilas[aux] || i == tamFilas[aux]) { // checa se todos os processos da fila foram finalizados, se não, recomeça a fila. 
                    i = 0;
                    continue;
                }
                if( filas[aux][i]->esta_rodando == 1 ) { // fazemos essa checagem para ver se o programa não foi iniciado
                    pid = fork();
                    if( pid < 0 ) {
                        printf("Fork com problemas na inicialização de processos da f%d\n",aux);
                    } else if( pid == 0 ) { // filho cria o programa
                        filas[aux][i]->esta_rodando = 0;
			filas[aux][i]->pid_processo = getpid();
                        execl( filas[aux][i]->nome, NULL, NULL);
                    } else {                // pai para o programa imediatamente
                        kill( pid, SIGSTOP);
                    }
                }
                
                if( filas[aux][i]->esta_rodando == 0 ) { // fazemos essa checagem para ver se o programa ja foi iniciado
                    printf("Processo %s em execuçao! ( Pid: %d )\n", filas[aux][i]->nome, filas[aux][i]->pid_processo);
                    kill( filas[aux][i]->pid_processo, SIGCONT); // Continua o processo
                    //sleep(quantum); //  Faz com que ele execute por quantum (s)
          				for(EVER){
							if(acabou == 0){
								
							}					
					}          


                    estadoProcesso = waitpid(filas[aux][i]->pid_processo, &status, WNOHANG);// Argumento WNOHANG faz com que a func wait pid retorne imediatamente se nenhum processo-filho terminou.
                    
                    if( estadoProcesso == 0 ) { // processo ainda não finalizado
                        printf("Processo %s pausado! ( Pid: %d )\n", filas[aux][i]->nome, filas[aux][i]->pid_processo);
                        kill( filas[aux][i]->pid_processo, SIGSTOP);
                    } else if( estadoProcesso == -1 ) { // processo finalizado com erro;
                        printf("Processo %s finalizado com erro! ( Pid: %d )\n", filas[aux][i]->nome, filas[aux][i]->pid_processo);
                    } else { // processo finalizado
                        printf("Processo %s finalizado com sucesso! ( Pid: %d )\n", filas[aux][i]->nome, filas[aux][i]->pid_processo);
                        if( aux == 0 || aux == 1 ) {
                            filas[aux+1][tamFilas[aux+1]+processosFinalizados] = filas[aux][i]; // se a fila for 1 ou 2, precisamos colocar o processo finalizado na fila de baixo
                        }
                        processosFinalizados++;
                    }
                }
            }
        }
    }
}

void processoFinalizado(int sig) {
    printf("Finalização de processo capturada\n");
}
