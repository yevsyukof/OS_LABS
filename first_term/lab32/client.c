#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_SOCKET_ADDR "server_socket"

char* buf = NULL;
int sockfd = -1;

void sigexit_handler() {
    free(buf);
    close(sockfd);
    exit(0);
}

int main() {

    sigset(SIGINT, sigexit_handler);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_un unix_addr;
    unix_addr.sun_family = AF_UNIX;

    strncpy(unix_addr.sun_path, SERVER_SOCKET_ADDR, sizeof(SERVER_SOCKET_ADDR));

    if (connect(sockfd, (struct sockaddr*) &unix_addr, sizeof(unix_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(2);
    }

    buf = malloc(BUFSIZ * sizeof(char));
    size_t readBytes;
    while ((readBytes = read(0, buf, BUFSIZ))) {
        write(sockfd, buf, readBytes);
    }
}

