#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int execvpe (char *file, char *argv[], char *envp[]) {
    extern char **environ;
    environ = envp;
    execvp(file, argv);
    return 2;
}

int main (int argc, char *argv[]){
//    char **ptr = envp;
//    while(*ptr != NULL) {
//        printf("%s\n", *ptr);
//        ++ptr;
//    }
    char *envp[] = {"author=Anatoly", "env=NEW", (char *) 0};
    execvpe(argv[1], &argv[1], envp);
    exit (1);
}

