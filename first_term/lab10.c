#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

extern int errno;

int main(int argc, char *argv[]) {
    int status;
    if (fork() == 0) {
        if (execvp(argv[1], &argv[1]) == -1) {
            exit(errno == ENOENT? 127 : 126); // 127 - файла проги не exists, 126 - другая ошибка
        }
    } else {
        wait(&status);
        printf("\nchild exit status: %d\n", WEXITSTATUS(status));
//        perror(strerror(WEXITSTATUS(status)));
        exit(0);
    }
}
