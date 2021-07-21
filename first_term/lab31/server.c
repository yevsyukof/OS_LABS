#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <poll.h>
#include <signal.h>

#define SERVER_SOCKET_ADDR "server_socket"

extern int errno;

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

struct server {
    struct pollfd listen_poll_fds[SOMAXCONN + 1];
    // для отслеживания изменений СЛУШАЮЩЕГО СОКЕТА и сокетов, сопоставляемых клиентским соединениям
    size_t listen_sock_cnt; // колво слушаемых на данный момент сокетов
    char *buf;
    size_t buf_size;
    size_t clients_to_remove_count;
    size_t first_client_to_remove;
};

void cleanupListenSocket(int sockfd) {
    close(sockfd);
    unlink(SERVER_SOCKET_ADDR);
}

int getServListenSocket(struct server *servDesc) {
    return servDesc->listen_poll_fds[0].fd;
}

int createServListenSocket() {
    int listen_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;

    strncpy(addr.sun_path, SERVER_SOCKET_ADDR, sizeof(SERVER_SOCKET_ADDR));

    if (bind(listen_sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        cleanupListenSocket(listen_sock_fd);
        exit(2);
    }

    if (listen(listen_sock_fd, SOMAXCONN) == -1) {
        perror("listen");
        cleanupListenSocket(listen_sock_fd);
        exit(3);
    }
    return listen_sock_fd;
}

void createServer(struct server *servDesc) {
    memset(servDesc, 0, sizeof(*servDesc));

    for (size_t i = 0; i < SOMAXCONN + 1; ++i) { // объявляем отслеживаемые действия у всех слушаемых сокетов
        servDesc->listen_poll_fds[i].events = POLLIN;
        servDesc->listen_poll_fds[i].fd = -1;
    }

    servDesc->listen_sock_cnt = 0;
    servDesc->listen_poll_fds[servDesc->listen_sock_cnt++].fd = createServListenSocket();

    servDesc->buf_size = BUFSIZ;
    servDesc->buf = (char*)malloc(servDesc->buf_size);
}

void closeServer(struct server *servDesc) {
    cleanupListenSocket(getServListenSocket(servDesc));
    free(servDesc->buf);
}

void markAsSocketToRemove(struct server *this, size_t removeSockIdx) {
    if (removeSockIdx == 0) {
        return;
    }

    if (this->listen_poll_fds[removeSockIdx].fd == -1) {
        return;
    }

    close(this->listen_poll_fds[removeSockIdx].fd);
    this->listen_poll_fds[removeSockIdx].fd = -1;
    this->clients_to_remove_count++;

    if (this->first_client_to_remove == 0 || this->first_client_to_remove > removeSockIdx) {
        this->first_client_to_remove = removeSockIdx;
    }
}

void removeDisconnectSockets(struct server *servDesc) {
    if (!servDesc->clients_to_remove_count) {
        return;
    }
    if (!servDesc->first_client_to_remove) {
        return;
    }

    size_t removed_count = 0;

    size_t j = servDesc->listen_sock_cnt - 1; // указывает на конец текущего списка слушаемых

    for (size_t i = servDesc->first_client_to_remove;
         removed_count < servDesc->clients_to_remove_count && i < j; ++i) {

        if (servDesc->listen_poll_fds[i].fd == -1) {
            while (servDesc->listen_poll_fds[j].fd == -1 && j > i) {
                j--;
                removed_count++;
            }
            swap(&servDesc->listen_poll_fds[i].fd, &servDesc->listen_poll_fds[j].fd);
            // переносим всех "действующих клиентов", окруженных удаленными, в начало списка
            // каждая -1 - говорит о том, что текущий клиент был удален
            j--;
            removed_count++;
        }
    }

    servDesc->listen_sock_cnt -= servDesc->clients_to_remove_count;

    servDesc->clients_to_remove_count = 0;
    servDesc->first_client_to_remove = 0;
}

void tryRead(struct server *servDesc, size_t readable_count) {
    size_t read_fd = 0;
    for (size_t curSockIdx = 1; read_fd <= readable_count && curSockIdx < servDesc->listen_sock_cnt; ++curSockIdx) {
        if (servDesc->listen_poll_fds[curSockIdx].revents & POLLIN) {
            read_fd++;

            size_t readBytes = read(servDesc->listen_poll_fds[curSockIdx].fd, servDesc->buf, servDesc->buf_size);
            for (size_t j = 0; j < readBytes; ++j) {
                servDesc->buf[j] = toupper(servDesc->buf[j]);
            }

            if (readBytes == -1) {
                perror("read");
                markAsSocketToRemove(servDesc, curSockIdx);
                continue;
            }
            write(1, servDesc->buf, readBytes);
            if (readBytes == 0) {
                markAsSocketToRemove(servDesc, curSockIdx);
            }
        }
    }
    removeDisconnectSockets(servDesc);
}

int exitFlag = 0;

void sigexit_handler() {
    exitFlag = 1;
}

int main() {
    struct server servDesc;
    createServer(&servDesc);

    sigset(SIGINT, sigexit_handler);

    size_t ready_fd_count;
    while ((ready_fd_count = poll(servDesc.listen_poll_fds, servDesc.listen_sock_cnt, -1)) != -1 && !exitFlag) {

        int hasConnectionRequests = servDesc.listen_poll_fds[0].revents & POLLIN;
        if (hasConnectionRequests) {
            int client_sockfd = accept(getServListenSocket(&servDesc), NULL, NULL);
            if (client_sockfd == -1) {
                perror("accept in while");
                closeServer(&servDesc);
                exit(13);
            }

            servDesc.listen_poll_fds[servDesc.listen_sock_cnt++].fd = client_sockfd;
        }

        tryRead(&servDesc, hasConnectionRequests ? ready_fd_count - 1 : ready_fd_count);
    }

    closeServer(&servDesc);
}

