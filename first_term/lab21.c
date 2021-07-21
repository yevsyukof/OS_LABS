#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// buferizacia po strokam ne ochen pri vivode

int count;
void sigbel_handler() {
    printf("%c\n", '\07');
    count++;
}

void sigexit_handler() {
    printf("\nCount = %d \n", count);
    exit(0);
}

int main() {
    sigset(SIGINT, sigbel_handler);
    sigset(SIGQUIT, sigexit_handler);
    while(1) {
        pause();
    }
}


