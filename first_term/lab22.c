#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#define TIME_OUT 5
#define MAX_FILES_CNT 10
#define LINE_LENGTH 28

extern int errno;

int outFileFd;

void writeLine(char *fileName, char *line) {
    char buf[256];
    sprintf(buf, "%s: %s\n", fileName, line); // заносим данные в массив
    write(outFileFd, buf, strlen(buf));
}

int wasAlarmSig;

void alarm_handler(int sig) {
    signal(sig, alarm_handler);
    wasAlarmSig = 1;
}

int cntOfFiles;
char *fileNames[MAX_FILES_CNT];
int fds[MAX_FILES_CNT];

void readFiles(void) {
    int i;
    int cntOpenFiles;
    char line[LINE_LENGTH + 1];

    signal(SIGALRM, alarm_handler);

    cntOpenFiles = cntOfFiles;
    while (cntOpenFiles) {
        for (i = 0; i < cntOfFiles; i++) {
            if (fds[i] == -1) {
                continue;
            }

            alarm(TIME_OUT);
            line[0] = '\0';
            int readBytes;
            // при прерывании read - возвращает кол-во прочитанных байт, либо -1, если ни одного не было прочит
            if ((readBytes = read(fds[i], line, LINE_LENGTH)) <= 0) {
                if (errno == EINTR && wasAlarmSig) {
                    wasAlarmSig = 0;
                } else {
                    close(fds[i]);
                    fds[i] = -1;  // close file
                    --cntOpenFiles;
                }
                continue;  // switch to the next file
            }
            alarm(0);
            line[readBytes] = '\0';
            writeLine(fileNames[i], line);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        perror("\nNot enough files\n");
        exit(1);
    }

    while (--argc) {
        fileNames[cntOfFiles] = argv[cntOfFiles + 1];
        if ((fds[cntOfFiles] = open(fileNames[cntOfFiles], O_RDONLY)) == -1) {
            fprintf(stderr, "\n%s: Cannot open file %s\n", argv[0], fileNames[cntOfFiles]);
            exit(2);
        }

        ++cntOfFiles;
        if (cntOfFiles >= MAX_FILES_CNT) {
            fprintf(stderr, "\n%s: Too many files\n", argv[0]);
            exit(3);
        }
    }

    outFileFd = open("out", O_WRONLY | O_APPEND);
    readFiles();
}

