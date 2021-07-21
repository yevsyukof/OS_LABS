#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <aio.h>
#include <signal.h>

#define SERVER_SOCKET_ADDR "server_socket"
#define AIO_READY_SIGNAL SIGUSR1

extern int errno;

void swap(struct aiocb *a, struct aiocb *b) {
    struct aiocb tmp = *a;
    *a = *b;
    *b = tmp;
}

struct server {
    struct aiocb clients[SOMAXCONN];
    // для отслеживания изменений СЛУШАЮЩЕГО СОКЕТА и сокетов, сопоставляемых клиентским соединениям
    int serv_listen_socket;
    int clients_cnt; // колво слушаемых клиентов на данный момент сокетов (указатель на свободное место)
    int buf_size; // размер буфера для каждого клиента
};
struct server *servDesc;
sigset_t aio_sig_mask;

void cleanupListenSocket(int sockfd) {
    close(sockfd);
    unlink(SERVER_SOCKET_ADDR);
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

void createServer() {
    memset(servDesc, 0, sizeof(*servDesc));

    servDesc->clients_cnt = 0;
    servDesc->serv_listen_socket = createServListenSocket();

    servDesc->buf_size = BUFSIZ;
    // дать людям памяти
    for (size_t i = 0; i < SOMAXCONN; ++i) {
        servDesc->clients[i].aio_fildes = -1;

        servDesc->clients[i].aio_nbytes = servDesc->buf_size; // размер буфера для описателя aio-запроса

        servDesc->clients[i].aio_sigevent.sigev_value.sival_int = i;
        // параметр, который будет передан обработчику сигнала при вызове (номер клиента в списке)

        servDesc->clients[i].aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        // способ уведомлению о завершении запроса (генерим сигнал)

        servDesc->clients[i].aio_sigevent.sigev_signo = AIO_READY_SIGNAL; // генерируемый сигнал
    }

    sigemptyset(&aio_sig_mask);
    // создаем "подмаску сигналов", которая поможет блокировать/разблокировать доставку сигналов на уровне ядра
    sigaddset(&aio_sig_mask, AIO_READY_SIGNAL);
}

void disconnectClients() {
    for (int i = 0; i < servDesc->clients_cnt; ++i) {
        close(servDesc->clients[i].aio_fildes);
        free((void*)servDesc->clients[i].aio_buf);
    }
}

void closeServer() {
    cleanupListenSocket(servDesc->serv_listen_socket);
    disconnectClients();
    free(servDesc);
}

void addClient(int client_sockfd) {
    sighold(AIO_READY_SIGNAL); // задерживаем доставку сигналов "на уровне процесса"

    int newClientIdx;
    for (int i = 0; i < SOMAXCONN; ++i) {
        if (servDesc->clients[i].aio_fildes == -1) {
            newClientIdx = i;
            break;
        }
    }
    servDesc->clients_cnt++;

    servDesc->clients[newClientIdx].aio_fildes = client_sockfd;
    servDesc->clients[newClientIdx].aio_buf = malloc(servDesc->buf_size); // даем новому клиенту буфер

    aio_read(&servDesc->clients[newClientIdx]);

    sigrelse(AIO_READY_SIGNAL);
}

void removeDisconnectClient(int clientIdx) {

    close(servDesc->clients[clientIdx].aio_fildes);
    servDesc->clients[clientIdx].aio_fildes = -1;
    free((void*)servDesc->clients[clientIdx].aio_buf);

//    if (clientIdx < servDesc->clients_cnt - 1) { // перемещаем в конец чела, который где-то посередине в списке
//        swap(&servDesc->clients[clientIdx], &servDesc->clients[servDesc->clients_cnt - 1]);
//    }
    servDesc->clients_cnt--;
}

void aiosig_handler(int signum, siginfo_t *recvSigInfo, void *unused) {
    sighold(AIO_READY_SIGNAL);

//    int clientRequestIdx = recvSigInfo->_sifields._rt.si_sigval.sival_int;

    union sigval clientRequest = recvSigInfo->si_value;
    int clientRequestIdx = clientRequest.sival_int;
    // здесь должен передаватся номер клиента, который посылает сигнал

    int clientIdx = clientRequestIdx;
//    for (int i = 0; i < servDesc->clients_cnt; ++i) {
//        if (servDesc->clients[i].aio_sigevent.sigev_value.sival_int == clientRequestIdx) {
//            clientIdx = i;
//            break;
//        }
//    }

    if (clientIdx == -1) {
        perror("aiosig_handlerPerror");
        closeServer();
        exit(1);
    }

    int readCnt = aio_return(&servDesc->clients[clientIdx]);

    if (readCnt == -1) {
        perror("aio_return");
        closeServer();
        exit(2);
    } else if (readCnt == 0) {
        removeDisconnectClient(clientIdx);
    } else {
        char *buf = (char*) servDesc->clients[clientIdx].aio_buf;
        for (size_t i = 0; i < readCnt; ++i) {
            buf[i] = toupper(buf[i]);
        }
        write(STDOUT_FILENO, buf, readCnt);
        aio_read(&servDesc->clients[clientIdx]);
    }

    sigrelse(AIO_READY_SIGNAL);
}

void sigexit_handler() {
    sigprocmask(SIG_BLOCK, &aio_sig_mask, NULL);
    // больше запросы не принимаем на уровне ядра (ведь у нас есть еще dispositions(disp))
    closeServer();
    exit(0);
}

int main() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO; // режим обработки сигнала
    act.sa_sigaction = aiosig_handler; // ссылка на обработчик сигнала
    sigaction(AIO_READY_SIGNAL, &act, NULL);

    servDesc = (struct server*) malloc(sizeof(struct server));
    createServer();

    sigset(SIGINT, sigexit_handler);

    while (1) {
        if (servDesc->clients_cnt < SOMAXCONN) {
            int client_fd = accept(servDesc->serv_listen_socket, NULL, NULL);

            if (client_fd == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("accept");
                closeServer();
                exit(1);
            }

            addClient(client_fd);
        }
    }
}

