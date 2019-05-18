#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int tempo = atoi(argv[0]);
    for( ;tempo > 0; tempo--) {
        printf("To no programa4 %d pid( %d )\n", tempo,getpid());
        sleep(1);
    }
    return 0;
}
