#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>

#define MAX_LINES_COUNT 25

typedef struct {
    char *host; // domain name or IP-address
    char *resource_path;
    int port;
} parsedURL;

parsedURL *parseURL(const char *url) {
    parsedURL *parsedUrl = (parsedURL*) malloc(sizeof(parsedURL));

    parsedUrl->host = (char*) malloc(100 * sizeof(char));
    parsedUrl->resource_path = (char*) calloc(100, sizeof(char));

    parsedUrl->port = 80;
    parsedUrl->resource_path[0] = '/';

    int isPortExists = strchr(url + 6, ':') != NULL;
    if (isPortExists) {
        sscanf(url, "http://%99[^:]:%99d%99[^\n]",
               parsedUrl->host, &parsedUrl->port, parsedUrl->resource_path);
    } else {
        sscanf(url, "http://%99[^/]%99[^\n]",
               parsedUrl->host, parsedUrl->resource_path);
    }

    printf("\nURL: %s\n", url);
    printf("Host: %s\n", parsedUrl->host);
    printf("Port: %d\n", parsedUrl->port);
    printf("resource_path: %s\n\n", parsedUrl->resource_path);
    return parsedUrl;
}

char *createHttpHeaders(parsedURL *parsedUrl) {
    int headersSize = strlen("GET  HTTP/1.1\r\n") + strlen(parsedUrl->resource_path) +
            strlen("Accept: text/html\r\n") +
            strlen("Host: \r\n") + strlen(parsedUrl->host) + 5;

    char *headers = (char*)calloc(headersSize, sizeof(char));

    strcat(headers, "GET ");
    strcat(headers, parsedUrl->resource_path);
    strcat(headers, " HTTP/1.1\r\n");

    strcat(headers, "Accept: text/html\r\n");

    strcat(headers, "Host: ");
    strcat(strcat(headers, parsedUrl->host), "\r\n\r\n");

    printf("-----HTTP REQUEST-----\n");
    printf("%s", headers);
    printf("----END OF REQUEST----\n\n");
    return headers;
}

void freeTmpAddrInfoStructs(struct addrinfo *hints, struct addrinfo *res) {
    free(hints);
    freeaddrinfo(res);
}

int createInteractionSockFd(const parsedURL *parsedUrl) {
    struct addrinfo *hints = calloc(1, sizeof(struct addrinfo)); 
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM; // TCP-sock

    hints->ai_flags = AI_NUMERICSERV; 
    char str_port[11];
    snprintf(str_port, 11, "%d", parsedUrl->port);

    struct addrinfo *res;
    if (getaddrinfo(parsedUrl->host, str_port, hints, &res)) {
        perror("getaddrinfo");
        free(hints);
        exit(1);
    }

    int interactionSockFd;
    if ((interactionSockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("socket");
        close(interactionSockFd);
        freeTmpAddrInfoStructs(hints, res);
        exit(2);
    }

    if (connect(interactionSockFd, res->ai_addr, res->ai_addrlen)) {
        perror("connect");
        close(interactionSockFd);
        freeTmpAddrInfoStructs(hints, res);
        exit(3);
    }

    freeTmpAddrInfoStructs(hints, res);
    return interactionSockFd;
}

struct termios saved_term_attributes;
void reset_term_attributes() {
    tcsetattr(fileno(stdin), TCSANOW, &saved_term_attributes);
}

void set_term_attributes() {
    struct termios new_attributes;
    memcpy(&new_attributes, &saved_term_attributes, sizeof(saved_term_attributes));
    new_attributes.c_lflag &= ~(ICANON | ECHO);
    new_attributes.c_cc[VMIN] = 1;
    tcsetattr(fileno(stdin), TCSANOW, &new_attributes);
}

void resetBufPart(char *buf, int *bufTop, int *cntWroteLines) {
    int outPartEndIdx = -1; 
    for (int i = 0; i <= *bufTop && *cntWroteLines < MAX_LINES_COUNT; ++i) {
        ++outPartEndIdx;
        if (buf[i] == '\n') {
            (*cntWroteLines)++;
        }
    }

    write(fileno(stdout), buf, outPartEndIdx + 1);
    if (outPartEndIdx < *bufTop) {
        memcpy(buf, buf + outPartEndIdx + 1, (*bufTop - outPartEndIdx) * sizeof(char));
    }
    *bufTop -= (outPartEndIdx + 1);
}

void http_client(const char *url) {
    parsedURL *parsedUrl = parseURL(url);

    int interactionSockFd = createInteractionSockFd(parsedUrl);

    char *headers = createHttpHeaders(parsedUrl);

    send(interactionSockFd, headers, strlen(headers), 0);

    tcgetattr(fileno(stdin), &saved_term_attributes);
    set_term_attributes();

    struct pollfd fds[2];
    fds[0].fd = fileno(stdin);
    fds[0].events = POLLIN;
    fds[1].fd = interactionSockFd;
    fds[1].events = POLLIN;

    int cntWroteLines = 0;
    char buf[BUFSIZ];
    int bufTop = -1; 
    int transmissionEndFlag = 0;
    while(!transmissionEndFlag || bufTop >= 0) {
        int ready_fd_count = poll(fds, 2, 0);

        if (ready_fd_count == -1) {
            perror("poll");
            close(interactionSockFd);
            reset_term_attributes();
            exit(6);
        }

        if (fds[1].revents & POLLIN && bufTop < BUFSIZ - 1 && !transmissionEndFlag) {
            int receivedData = recv(interactionSockFd, buf + bufTop + 1, BUFSIZ - (bufTop + 1), 0);

//            if (receivedData == 0 || bufTop < 0) {
//                transmissionEndFlag = 1;
//            }

            if (receivedData == 0) {
                transmissionEndFlag = 1;
            }

            if (receivedData == -1) {
                perror("recv");
                close(interactionSockFd);
                reset_term_attributes();
                exit(7);
            }
            bufTop += receivedData;
        }

        if (bufTop >= 0 && cntWroteLines < MAX_LINES_COUNT) {

            resetBufPart(buf, &bufTop, &cntWroteLines);

            if (cntWroteLines == MAX_LINES_COUNT) {
                printf("\npress SPACE to scroll\n");
            }
        }

        char sym;
        if (fds[0].revents & POLLIN && read(fileno(stdin), &sym, 1) && sym == ' ') {
            cntWroteLines = 0;
        }
    }

    printf("\n\nEND!!!\n\n");
    close(interactionSockFd);
    reset_term_attributes();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("too few arguments\n");
        exit(0);
    }

    http_client(argv[1]);
}

