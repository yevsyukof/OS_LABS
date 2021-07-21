#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    struct flock lock;
    int fileDesc;
    char command[100];

    if ((fileDesc = open(argv[1], O_RDWR)) == -1) {
        perror(argv[1]);
        exit(1);
    }
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fileDesc, F_SETLK, &lock) == -1) {
        if ((errno == EACCES) || (errno == EAGAIN)) {
            printf("%s busy -- try later\n", argv[1]);
            exit(2);
        }
        perror(argv[1]);
        exit(3);
    }
    sprintf(command, "nano %s\n", argv[1]);
    system(command);

    lock.l_type = F_UNLCK;    /* unlock file */
    fcntl(fileDesc, F_SETLK, &lock);
    close(fileDesc);
    return 0;
}

