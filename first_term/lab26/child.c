#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char input[1000];
    int i;
    int readCnt;
    while ((readCnt = read(0, input, 1000)) > 0) {
        for (i = 0; i < readCnt; i++) {
            if (islower(input[i])) {
                input[i] = toupper(input[i]);
            }
        }
        write(1, input, readCnt);
    }
    exit(0);
}

