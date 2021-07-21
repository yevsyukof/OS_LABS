#include <stdio.h>

int main(int argc, char **argv) {
    FILE *file, *pipe;
    char line[BUFSIZ];

    file = fopen(argv[1], "r");

    pipe = popen("wc -l", "w");
    while (fgets(line, BUFSIZ, file) != (char *) NULL) {
        if (line[0] == '\n') {
            fputs(line, pipe);
        }
    }
    fclose(file);
    pclose(pipe);
}

