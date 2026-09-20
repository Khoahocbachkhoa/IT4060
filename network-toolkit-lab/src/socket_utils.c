#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "socket_utils.h"

void send_cmd(int sockfd, const char *cmd) {
    send(sockfd, cmd, strlen(cmd), 0);
}

int recv_response(int sockfd, char *buf, int sz) {
    int n = recv(sockfd, buf, sz - 1, 0);
    if (n < 0) {
        perror("read error");
        return -1;
    }

    buf[n] = '\0';
    // #printf("Receive response: %s", buf);
    return n;
}

int connect_to_server(const char *ip, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "cannot connect to server\n");
        return -1;
    }

    return sockfd;
}

int send_all(int fd, const char *buf) {
    int len = strlen(buf);

    while (len > 0) {
        int n = send(fd, buf, len, 0);

        if (n == 0) {/* server disconnect */
            fprintf(stderr, "Lost connection to server!\n");
            return -1;
        } else if (n < 0) {
            perror("send error\n");
            return -1;
        }

        buf += n;
        len -= n;
    }

    return 0;
}