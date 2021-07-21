#include <stdio.h>

int main() {
    FILE *fptr;
    char *line = {"ThIsIsThEoRiGiNaLtExT\n"};
    fptr = popen("./child", "w");
    fputs(line, fptr);
    pclose(fptr);
}

