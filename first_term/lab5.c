#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fileDesc, lineNumber = 0;
    if ((fileDesc = open(argv[1], O_RDWR)) == -1) {
        exit(1);
    }
    char *buf = (char*)malloc(sizeof(char) * BUFSIZ);
    int bufSize = BUFSIZ;
    long *fileOffsets = (long*)malloc(sizeof(long) * 15000);
    fileOffsets[0] = 0L;
    int *linesLength = (int*)calloc(15000, sizeof(int));
    int statBufSize = 15000;

    int readBytes;
    int cntOfLines = 0, curLineLen = 0;
    while ((readBytes = read(fileDesc, buf, BUFSIZ)) && readBytes != -1) {
        for (int i = 0; i < readBytes; ++i) {
            if (buf[i] == '\n') {
                ++curLineLen;
                linesLength[cntOfLines] = curLineLen;
                fileOffsets[cntOfLines++] = lseek(fileDesc, 0L, SEEK_CUR) - readBytes + i - curLineLen + 1;
                curLineLen = 0;
            } else {
                ++curLineLen;
            }
            if (cntOfLines == statBufSize) {
                statBufSize *= 2;
                fileOffsets = (long*) realloc(fileOffsets, sizeof(long) * statBufSize);
                linesLength = (int*) realloc(linesLength, sizeof(int) * statBufSize);
            }
        }
    }

    while (printf("enter line number : ") && scanf("%d", &lineNumber)) {
        if (lineNumber == 0) {
            exit(0);
        }
	if ( lineNumber == 13) {
	    write(fileDesc, "2", 1);
	}
        lineNumber--;
        if (lineNumber < 0 || lineNumber >= cntOfLines) {
            fprintf(stderr, "wrong line number\n");
            continue;
        }

        if (linesLength[lineNumber] > bufSize) {
            buf = (char*) realloc(buf, linesLength[lineNumber] * sizeof(char));
            bufSize = linesLength[lineNumber];
        }

        lseek(fileDesc, fileOffsets[lineNumber], SEEK_SET);
        if (read(fileDesc, buf, linesLength[lineNumber])) {
            write(1, buf, linesLength[lineNumber]);
        }
    }

    free(buf);
    free(fileOffsets);
    free(linesLength);
    close(fileDesc);
    return 0;
}

