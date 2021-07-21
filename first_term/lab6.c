#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFERSIZE 1000

int main(int argc, char *argv[]) {
    int fileDesc, lineNumber = 0;
    if ((fileDesc = open(argv[1], O_RDONLY)) == -1) {
        exit(1);
    }
    char *buf = (char *) malloc(sizeof(char) * BUFERSIZE);
    int bufSize = BUFERSIZE;
    long *fileOffsets = (long *) malloc(sizeof(long) * 15000);
    fileOffsets[0] = 0L;
    int *linesLength = (int *) calloc(15000, sizeof(int));
    int statBufSize = 15000;

    int readBytes;
    int cntOfLines = 0, curLineLen = 0;
    while ((readBytes = read(fileDesc, buf, BUFERSIZE)) && readBytes != -1) {
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
                fileOffsets = (long *) realloc(fileOffsets, sizeof(long) * statBufSize);
                linesLength = (int *) realloc(linesLength, sizeof(int) * statBufSize);
            }
        }
    }

    int terminalFD = open("/dev/tty", O_RDWR | O_NDELAY);
    if (terminalFD == -1) {
        perror("terminal open error\n");
        exit(1);
    }

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    time_t beginTime = currentTime.tv_sec;
    printf("you have five seconds to write a line number\n");
    while (1) {
        gettimeofday(&currentTime, NULL);
        if (currentTime.tv_sec - beginTime >= 5) {
            lseek(fileDesc, 0, SEEK_SET);
            while ((readBytes = read(fileDesc, buf, bufSize)) > 0) {
                write(terminalFD, buf, readBytes);
            }
            break;
        } else if ((readBytes = read(terminalFD, buf, bufSize)) > 0) {
            buf[readBytes] = '\n';
            lineNumber = atoi(buf);
            if (lineNumber == 0) {
                exit(0);
            }
            lineNumber--;
            if (lineNumber < 0 || lineNumber >= cntOfLines) {
                fprintf(stderr, "wrong line number\n");
                continue;
            }

            if (linesLength[lineNumber] > bufSize) {
                buf = (char *) realloc(buf, linesLength[lineNumber] * sizeof(char));
                bufSize = linesLength[lineNumber];
            }

            lseek(fileDesc, fileOffsets[lineNumber], SEEK_SET);
            if (read(fileDesc, buf, linesLength[lineNumber])) {
                write(1, buf, linesLength[lineNumber]);
            }
            beginTime = currentTime.tv_sec;
        }
    }

    free(buf);
    free(fileOffsets);
    free(linesLength);
    close(fileDesc);
    close(terminalFD);
    return 0;
}

