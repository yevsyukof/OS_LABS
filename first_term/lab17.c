#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <memory.h>

#define BUFF_SIZE 40

struct termios saved_attributes;

void reset_term_attributes() {
    tcsetattr(0, TCSANOW, &saved_attributes);
}

void set_term_attributes() {
    struct termios new_attributes;
    if (!isatty(0)) {
        fprintf(stderr, "Not a terminal 'file'.\n");
        exit(1);
    }
    tcgetattr(0, &saved_attributes);
    memcpy(&new_attributes, &saved_attributes, sizeof(new_attributes));
    new_attributes.c_lflag &= ~(ICANON | ECHO);
    new_attributes.c_cc[VMIN] = 1;
    tcsetattr(0, TCSAFLUSH, &new_attributes);
}

void delete_symbol(int *iter) {
    if (*iter != 0) {
        write(1, "\b \b", 3); //смещение позиции на шаг назад печать символом \b
        (*iter)--;
    }
}

void delete_word(char *buffer, int *iter) {
    while ((*iter != 0) && (buffer[*iter - 1] == ' ')) { // все пробелы перед словом
        delete_symbol(iter);
    }
    while ((*iter != 0) && (buffer[*iter - 1] != ' ')) { // слово
        delete_symbol(iter);
    }
    while ((*iter != 0) && (buffer[*iter - 1] == ' ')) { // пробелы после
        delete_symbol(iter);
    }
}

void delete_string(int *iter) {
    while (*iter != 0) {
        delete_symbol(iter);
    }
}

int main() {
    set_term_attributes();
    atexit(reset_term_attributes);
    int iter = 0;
    char buffer[BUFF_SIZE];
    char nextChar;
    while (1) {
        read(0, &nextChar, 1);
        switch (nextChar) {
            case CWERASE: {
                delete_word(buffer, &iter);
                break;
            }
            case CKILL: {
                delete_string(&iter);
                break;
            }
            case CERASE: {
                delete_symbol(&iter);
                break;
            }
            case CEOF: {
                if (iter == 0) {
                    return 0;
                }
                break;
            }
            default: {
                buffer[iter] = nextChar;
                ++iter;
                write(1, &nextChar, 1);
                write(1, "\07", 1); // звуковой сигнал
                if (iter == BUFF_SIZE) {
                    write(1, "\n", 1);
                    iter = 0;
                }
                break;
            }
        }
    }
}

