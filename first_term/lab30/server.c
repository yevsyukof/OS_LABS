#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#define SERVER_SOCKET_ADDR "server_socket"

void printUpperText(int interactionSockFd) {
    char* buf = malloc(BUFSIZ * sizeof(char));
    size_t readBytes;
    while ((readBytes = read(interactionSockFd, buf, BUFSIZ))) {
        for (size_t i = 0; i < readBytes; ++i) {
            buf[i] = toupper(buf[i]);
        }
        write(1, buf, readBytes);
    }
    free(buf);
}

void cleanupListenSocket(int sockfd) {
    close(sockfd);
    unlink(SERVER_SOCKET_ADDR); // удаляем файл, созданный для взаимодействия
}

int main() {
    int listen_sock_fd;

    listen_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un servAddr;
    servAddr.sun_family = AF_UNIX;
    strncpy(servAddr.sun_path, SERVER_SOCKET_ADDR, sizeof(SERVER_SOCKET_ADDR));

    if (bind(listen_sock_fd, (struct sockaddr*) &servAddr, sizeof(servAddr)) == -1) {
        perror("bind");
        cleanupListenSocket(listen_sock_fd);
        exit(2);
    }

    if (listen(listen_sock_fd, 1) == -1) {
        perror("listen");
        cleanupListenSocket(listen_sock_fd);
        exit(3);
    }

    int interactionSockFd = accept(listen_sock_fd, NULL, NULL);
    if (interactionSockFd == -1) {
        perror("accept");
        exit(4);
    }

    printUpperText(interactionSockFd);
    close(interactionSockFd);
    cleanupListenSocket(listen_sock_fd);
}

