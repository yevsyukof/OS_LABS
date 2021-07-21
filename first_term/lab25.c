#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int pid;
    int fd[2];
    static char *lines = {"PaRent_TeXt\n"};
    char input[1000];
    int readCnt;
    pipe(fd);
    if ((pid = fork()) > 0) { // parent case
        close(fd[0]);
        write(fd[1], lines, strlen(lines));
        close(fd[1]);
    } else if (pid == 0) { // child case
        close(fd[1]);
        while ((readCnt = read(fd[0], input, 1000)) > 0) {
            for (int i = 0; i < readCnt; ++i) {
                if (islower(input[i])) {
                    input[i] = toupper(input[i]);
                }
            }
            write(1, input, readCnt);
        }
        close(fd[0]);
    }
    exit(0);
}

