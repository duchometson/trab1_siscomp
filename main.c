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
int flagAlarmeTocando = 1; // false = 1 , true = 0;
int devoTocarAlarme = 0; // false = 1 , true = 0;
int processosFinalizados = 0;
int fila = 0;

struct programa {
    int pid_processo;
    int me_pule ; // nao pule = 1 , pule = 0
    char nome[10];
    short int esta_rodando; // não esta  = 1 , esta = 0; finalizado = 3
    int tempoRajadas[3];
    int idRajada;// primeira rajada = 0 , segunda rajada = 1 , terceira rajada = 2
}; typedef struct programa Programa;

void pausaProcesso();
void continuaProcesso();
void processoFinalizado();
void alarme();
int escalonador( Programa *F1, Programa *F2, Programa *F3, int *numeroProgramasLidos, int *statusInterpretador);
int interpretador ( FILE *arqExec, Programa *F1, Programa *F2, Programa *F3, int *numeroProgramasLidos, int *statusInterpretador );
int defineQuantumPorFila( int fila );
void jogaProFinaleArruma( Programa *fila, int corrente, int tamFila );


int main(int argc, char *argv[]) {

    Programa *F1;
    Programa *F2;
    Programa *F3;
    int *numeroProgramasLidos;
    int *statusInterpretador;
    
    int status;
    int segmentoFila1, segmentoFila2, segmentoFila3;
    int segmentonumeroProgramasLidos;
    int segmentoStatusInterpretador;
    int pid; 
    ///// Criação de Espaço de memória para filas do escalonador///////
    segmentoFila1 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F1 = (Programa*)shmat(segmentoFila1,0,0);
     
    segmentoFila2 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F2 = (Programa*)shmat(segmentoFila2,0,0);
     
    segmentoFila3 = shmget (IPC_PRIVATE, sizeof (Programa) * LIMITE_PROGRAMAS_FILA, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    F3 = (Programa*)shmat(segmentoFila3,0,0);
    
    
    signal(SIGALRM, alarme);
    
                                                                
    segmentonumeroProgramasLidos = shmget (IPC_PRIVATE, sizeof (int) , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    numeroProgramasLidos = (int*)shmat(segmentonumeroProgramasLidos,0,0);
    
    segmentoStatusInterpretador = shmget (IPC_PRIVATE, sizeof (int) , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    statusInterpretador = (int*)shmat(segmentoStatusInterpretador,0,0);

    (*numeroProgramasLidos) = 0;
    (*statusInterpretador) = 1;
    
    FILE  *arqExec = fopen("exec.txt", "r");

    pid = fork();
    
    if( pid == 0 ) {                         // Filho
        interpretador( arqExec, F1, F2, F3, numeroProgramasLidos, statusInterpretador);
    } else {                                 // Pai
        waitpid(pid, &status, WCONTINUED);   // Espera a leitura do interpretador
        escalonador(F1, F2, F3, numeroProgramasLidos, statusInterpretador);         
    }
     
    
    printf("Fim.\n");
    shmdt(F1);
    shmdt(F2);
    shmdt(F3);
    shmdt(numeroProgramasLidos);
    shmdt(statusInterpretador);
    
}

int interpretador ( FILE *arqExec, Programa *F1, Programa *F2, Programa *F3, int *numeroProgramasLidos, int *statusInterpretador )
{
    // Arquivo tera linhas com o seguinte formato:
    // exec programa1 (2,10,4)
    char exec[5];    // comando exec
    char nomePrograma[10]; // nome do programa - 10 caracteres de limite, eh necessario mais? 
    int t1, t2, t3;  // t1 tempo da primeira rajada, t2 da segunda e t3 da terceira.     

    (*statusInterpretador) = 0; // interpretador esta rodando 
    (*numeroProgramasLidos) = 0;
    int  i = 0;
    
    while( (fscanf(arqExec,"%s %s (%d,%d,%d)\n", exec, nomePrograma, &t1, &t2, &t3)) != EOF ) {
        strcpy(F1[i].nome, nomePrograma);
        F1[i].tempoRajadas[0] = t1;
        F1[i].tempoRajadas[1] = t2;
        F1[i].tempoRajadas[2] = t3;
        F1[i].esta_rodando = 1;
        F1[i].idRajada = 0;
        (*numeroProgramasLidos)++;
        i++;
        
        printf(" LINHAS LIDA: %s %s (%d,%d,%d)\n", exec, nomePrograma, t1, t2, t3);
    }
    printf("%d prog lidos\n", (*numeroProgramasLidos));
    (*statusInterpretador) = 1; // interpretador terminou
    exit(0);
}

int escalonador( Programa *F1, Programa *F2, Programa *F3, int *numeroProgramasLidos, int *statusInterpretador) {
    int i = 0, pid, status;
    int quantum = 3;
    int tamF1 = 0, tamF2 = 0, tamF3 = 0;
    int tempF1 = 0, tempF2 = 0, tempF3 = 0;
    int ultimaPosF1 = 0, ultimaPosF2 = 0, ultimaPosF3 = 0;  
    int tamFilas[3];
    Programa *filas[3];
    
    int estadoProcesso;
	int rajada_corrente = 0;
    
    filas[0] = F1;
    filas[1] = F2;
    filas[2] = F3;
    
    signal(SIGCHLD, processoFinalizado); // cadastra o catcher de quando um processo filho é finalizado

    alarm(defineQuantumPorFila(0));
    
    tamF1 = (*numeroProgramasLidos);

    while( processosFinalizados < 3 * (*numeroProgramasLidos) ) { // enquanto ainda existirem processos para ser lidos ou existentes
        while( fila < 3 ) {
            printf("///////////Estou na fila %d//////////////// \n", fila+1);
            alarm(defineQuantumPorFila(fila));
            if(( fila == 0 && tamF1 == 0 ) || ( fila == 1 && tamF2 == 0 ) || ( fila == 2 && tamF3 == 0 )) { // checa se existem processos na fila corrente
                printf("Fila %d vazia!\n",fila+1);
                fila++;
                continue;
            }
            
            tempF1 = tamF1;
            tempF2 = tamF2;
            tempF3 = tamF3;
            while( (fila == 0 && tamF1 > i)  || (fila == 1 && tamF2 > i) || (fila == 2 && tamF3 > i) ) { // tratamento para a filas individualmente
              //  printf("i = %d\n",i);
//                 printf("tamF1 = %d\n", tempF1);
//                 printf("tamF2 = %d\n", tempF2);
//                 printf("tamF3 = %d\n", tempF3);
                if( filas[fila][0].idRajada > 2 ) { // checa se todos os processos da fila foram finalizados
                    if( fila == 0 ) tempF1--;
                    else if( fila == 1 ) tempF2--;
                    else tempF3--;
                    i++;
                    break;
                }
                if( filas[fila][0].esta_rodando == 1 ) { // fazemos essa checagem para ver se o programa não foi iniciado
                    pid = fork();
                    if( pid < 0 ) {
                        printf("Fork com problemas na inicialização de processos da f%d\n",fila);
                    } else if( pid == 0 ) { // filho cria o programa
                        printf("Criando processo %s\n", filas[fila][0].nome);
                        char buffer[50];
                        sprintf( buffer, "%d", filas[fila][0].tempoRajadas[filas[fila][0].idRajada]);
                        char *args[] = {  buffer, NULL  };
                        execv( filas[fila][0].nome, args);
                    } else {                // pai para o programa imediatamente
                        filas[fila][0].esta_rodando = 0;
                        filas[fila][0].me_pule = 1;
                        filas[fila][0].pid_processo = pid;
                        kill( filas[fila][0].pid_processo, SIGSTOP);
                    }
                }
                if( filas[fila][0].esta_rodando == 0 ) { // fazemos essa checagem para ver se o programa ja foi iniciado
                    if( filas[fila][0].me_pule == 0 ) {
                        i++;
                        filas[fila][0].me_pule = 1;
                        continue;
                    }
                    printf("%s em execuçao! Rajada de número: %d Vou durar: %d ( Pid: %d )\n", filas[fila][0].nome, filas[fila][0].idRajada + 1, filas[fila][0].tempoRajadas[filas[fila][0].idRajada],filas[fila][0].pid_processo);
                    kill( filas[fila][0].pid_processo, SIGCONT); // Continua o processo
                    for(EVER){
                        
                        estadoProcesso = waitpid(filas[fila][0].pid_processo, &status, WNOHANG);// Argumento WNOHANG faz com que a func wait pid retorne imediatamente se nenhum processo-filho terminou.
                        
                        if(flagAlarmeTocando == 0) { // alarme disparou antes do final do processo
                            kill( filas[fila][0].pid_processo, SIGSTOP);
                            flagAlarmeTocando = 1;
                            // se a fila for 1 ou 2, precisamos colocar o processo finalizado na fila de baixo
                            filas[fila][0].tempoRajadas[filas[fila][0].idRajada] = filas[fila][0].tempoRajadas[filas[fila][0].idRajada] - defineQuantumPorFila(fila);
                            if( filas[fila][0].tempoRajadas[filas[fila][0].idRajada] <= 0 ) { // se processo acabar no instante do alarme
                                processosFinalizados++;
                                filas[fila][0].esta_rodando = 1;
                                if( filas[fila][0].idRajada == 2 ) { // checa se todos os processos da fila foram finalizados
                                    if( fila == 0 ) {
                                        jogaProFinaleArruma( filas[fila], 0, tempF1 );
                                        tempF1--;
                                    }
                                    else if( fila == 1 ) {
                                        jogaProFinaleArruma( filas[fila], 0, tempF2 );
                                        tempF2--;
                                    }
                                    else { 
                                        jogaProFinaleArruma( filas[fila], 0, tempF3 );
                                        tempF3--;
                                    }
                                    i++;
                                    break;
                                }
                                filas[fila][0].idRajada++;
                                if ( fila == 0 ) { // mesma fila
	                                 printf("Colocando f%d de posicao 0 na f%d posicao %d\n",fila+1,fila+1,tempF1);
                                    jogaProFinaleArruma( filas[fila], 0, tempF1 );
                       
                                } else if( fila == 1 ) {
                                     printf("Colocando f%d de posicao 0 na f%d posicao %d\n",fila+1,fila,tempF2);
                                    filas[fila-1][tempF1] = filas[fila][0];
                                    jogaProFinaleArruma( filas[fila], 0, tempF2 );
                                    tempF2--;
                                    tempF1++;
                                } else {
                                     printf("Colocando f%d de posicao 0 na f%d posicao %d\n",fila+1,fila,tempF3);
                                    filas[fila-1][tempF2] = filas[fila][0];
                                    jogaProFinaleArruma( filas[fila], 0, tempF3 );
                                    tempF3--;
                                    tempF2++;
                                }
                             
                            } else {
                                if( fila == 0 ) {
                                     printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila+2,tempF2);
                                    filas[fila+1][tempF2] = filas[fila][0];
                                    jogaProFinaleArruma( filas[fila], 0, tempF1 );
                                    //filas[fila][i].me_pule = 0;                                
                                    tempF1--;
                                    tempF2++;
                                } else if( fila == 1 ) {
                                     printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila+2,tempF3);
                                    filas[fila+1][tempF3] = filas[fila][0];
                                    jogaProFinaleArruma( filas[fila], 0, tempF2 );
                                                                
                                    tempF2 --;
                                    tempF3 ++;
                                } else { // se algum processo da fila f3 não terminar e for ao final da fila.
                                     printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila+1,tempF3);
                                    jogaProFinaleArruma( filas[fila], 0, tempF3 );
                                    i++;
                                    
                                }
                            }
                            break;
						}
						if( estadoProcesso > 0 ) { // processo terminou antes do quantum
                             //printf("Estado do processo atual = %d\n", estadoProcesso);
                             printf("Terminei antes do quantum - Vou dormir\n");
                            
                            alarm(10);
                            for(int j = 0; j<3; j++) {
                                printf("Bloqueado por I/O- dormindo...\n");
                                sleep(1);
                            }
                            alarm(defineQuantumPorFila(fila)); //comeca um novo processo
                            processosFinalizados++;
                            filas[fila][0].esta_rodando = 1;
                            if( filas[fila][0].idRajada == 2 ) { // checa se todos os processos da fila foram finalizados
                                if( fila == 0 ) {
                                    jogaProFinaleArruma( filas[fila], 0, tempF1 );
                                    tempF1--;
                                }
                                else if( fila == 1 ) {
                                    jogaProFinaleArruma( filas[fila], 0, tempF2 );
                                    tempF2--;
                                }
                                else { 
                                    jogaProFinaleArruma( filas[fila], 0, tempF3 );
                                    tempF3--;
                                }
                                i++;
                                break;
                            }
                            filas[fila][0].idRajada++;
                            if( fila == 1 ) {
                                 printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila,tempF1);
                                filas[fila-1][tempF1] = filas[fila][0];
                                jogaProFinaleArruma( filas[fila], 0, tempF2 );
                              
                                tempF1++;
                                tempF2--;
                            }
                            else if ( fila == 2 ) {
                                 printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila,tempF2);
                                filas[fila-1][tempF2] = filas[fila][0];
                                jogaProFinaleArruma( filas[fila], 0, tempF3 );
                                //filas[fila][i].me_pule = 0;
                                tempF2 ++;
                                tempF3 --;
                            } else {
                                  printf("Colocando f%d de posicao %d na f%d posicao %d\n",fila+1,i,fila+1,tempF1);
                                jogaProFinaleArruma( filas[fila], 0, tempF1 );
                                i++;
                               								
                            }
                            break; //
                        }
                    }                   
                }
                i++;
                 printf(" $$$Processos Finalizados: %d \n",processosFinalizados); 
            }
            i = 0;
            fila++;
            tamF1 = tempF1;
            tamF2 = tempF2;
            tamF3 = tempF3;
        }
        fila = 0;
    }
    printf("Programas Existentes: %d Processos Finalizados: %d \n", (*numeroProgramasLidos), processosFinalizados);
    return 0;
}

void jogaProFinaleArruma( Programa *fila, int corrente, int tamFila ) {
    int i = corrente;
    Programa aux;
    if( tamFila > 1 ) {
        aux = fila[corrente];
        while( i < tamFila-1 ) {
            fila[i] = fila[i+1];
            i++;
        }
        fila[tamFila-1] = aux;
    }
}

int defineQuantumPorFila( int fileira ) {
    if( fileira == 0 ) {
        return 1;
    } else if( fileira == 1 ) {
        return 2;        
    } else if( fileira == 2 ) {
        return 4;
    }
}
    

void processoFinalizado(int sig) {
    //printf("Finalização de processo capturada %d\n", sig);
}

void alarme() {
    printf("Toquei o alarme\n");
    alarm(defineQuantumPorFila(fila)); 
    flagAlarmeTocando = 0;
}
