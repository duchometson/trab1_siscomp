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

struct programa {
    int pid_processo;
    int prioridade;
    char nome[10];
    int rodando;
    
}; typedef struct programa Programa;

void pausaProcesso();
void continuaProcesso();
void eliminaProcesso();
int escalonador(Programa *F1, Programa *F2, Programa *F3);

int main(void) {
    Programa *F1;
    Programa *F2;
    Programa *F3;
    
    //signal(SIGSTOP, pausaProcesso);  sigstop não pode ser capturado
    //signal(SIGCONT, continuaProcesso); como sigstop não pode ser capturado não precisaremos capturar esse sinal tb.
    signal(SIGCHD, eliminaProcesso);
    
}


int escalonador( Programa *F1, Programa *F2, Programa *F3) {
    while( F1
}
