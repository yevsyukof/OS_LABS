#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>


int main(int argc, char **argv) {
    pid_t pid, ret;
    int ccp = 0;
    if (argc < 2) {
        printf("invalid count of arguments!\n");
        return 0;
    }
    if ((pid = fork()) == 0) { // child case
        sleep(4);
        execl("/bin/cat", "cat", argv[1], (char *) 0);
        // execl executes cat on forked process and after that process dies
        // if we still here its very bad (-_-)
    } else { // parent case
        printf("parent: begin waiting for child with PID = %d\n", pid);
    }
    ret = wait(&ccp);
    printf("parent: wait's return value: %d,", ret);
    printf("child's ccp: %d\n", WEXITSTATUS(ccp));
}

