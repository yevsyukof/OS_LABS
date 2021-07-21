#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int setAttrs(int desc, struct termios oldAttrs) {
    oldAttrs.c_lflag &= ~(ICANON);
    oldAttrs.c_cc[VMIN] = 1;
    oldAttrs.c_cc[VTIME] = 0;
    if (tcsetattr(desc, TCSANOW, &oldAttrs) == -1) {
        perror("tcsetattr failed");
        return -1;
    }
    return 0;
}

int ask(int desc) {
    char *askLine = "Do you write symbol here?";
    char inputSymbols[1];
    write(desc, askLine, strlen(askLine));
    read(desc, inputSymbols, 1);
    write(desc, "\n", 1);
    write(desc, "Good Answer!", 12);
    write(desc, "\n", 1);
    return 0;
}

int main(int argc, char *argv[]) {
    int desc = open("/dev/tty", O_RDWR);
    if (desc == -1) {
        perror("open failed");
        return -1;
    }

    if (isatty(desc) == 0) {
        perror("not terminal opened");
        close(desc);
        return -1;
    }

    struct termios oldAttrs;
    if (tcgetattr(desc, &oldAttrs) == -1) {
        perror("tcgetattr failed");
        close(desc);
        return -1;
    }

    if (setAttrs(desc, oldAttrs) == -1) {
        close(desc);
        return -1;
    }

    int askReturnValue = ask(desc);
    if (tcsetattr(desc, TCSANOW, &oldAttrs) == -1) {
        perror("tcsetattr failed");
        close(desc);
        return -1;
    }
    close(desc);
    return askReturnValue;
}

