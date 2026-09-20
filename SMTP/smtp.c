#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "smtp.h"

#define MAXLINE 1024

int connect_to_server(const char *ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    int ret = inet_pton(AF_INET, ip, &saddr.sin_addr);

    if (ret == 0) {
        fprintf(stderr, "Wrong ip format\n");
        return -1;
    } else if (ret < 0) {
        perror("inet_pton error");
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("connect error");
        return -1;
    }

    char res[256];

    if (recv_response(fd, res, sizeof(res)) < 0)
        return -1;

    if (!check_response_code(res, "220"))
        return -1;

    return fd;
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

int recv_response(int fd, char *buf, size_t buf_size) {
    size_t len = 0;

    while (len < buf_size - 1) {
        int n = recv(fd, buf + len, buf_size - len - 1, 0);

        if (n == 0) {
            fprintf(stderr, "Lost connection to server\n");
            return -1;
        } else if (n < 0) {
            perror("recv error\n");
            return -1;
        }

        len += n;

        if (len >= 2 && buf[len-2] == '\r' && buf[len-1] == '\n') {
            break;
        }
    }
    
    buf[len] = '\0';
    return len;
}


int check_response_code(const char *res, const char *expected_code) {
    char code[8];

    sscanf(res, "%s", code);

    if (strcmp(code, expected_code) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int send_cmd(int fd, const char *cmd, const char *expected_code) {
    char res[256];

    if (send_all(fd, cmd) < 0) {
        return -1;
    }

    if (recv_response(fd, res, 256) < 0) {
        return -1;
    }

    if (!check_response_code(res, expected_code)) {
        fprintf(stderr, "Unexpected response code!\n");
        return -1;
    }

    return 0;
}

int send_header(int fd, const char *from, const char *to, const char *sub) {
    char line[MAXLINE];
    snprintf(line, MAXLINE, "From: %s\r\n", from);
    
    if (send_all(fd, line) < 0)
        return -1;

    
    snprintf(line, MAXLINE, "To: %s\r\n", to);
    
    if (send_all(fd, line) < 0)
        return -1;

    snprintf(line, MAXLINE, "Subject: %s\r\n\r\n", sub);
    
    if (send_all(fd, line) < 0)
        return -1;

    return 0;
}

int read_email_body_and_send(int fd) {
    char line[MAXLINE + 1];
    char buf[MAXLINE + 4];
    char res[256];

    while (fgets(line, MAXLINE, stdin)) {
        line[strcspn(line, "\r\n")] = '\0';
        
        snprintf(buf, sizeof(buf), "%s\r\n", line);

        if (send_all(fd, buf) < 0)
            return -1;
    }

    // end of data
    if (send_all(fd, ".\r\n") < 0)
        return -1;

    if (recv_response(fd, res, 256) < 0)
        return -1;
    
    if (!check_response_code(res, "250")) {
        fprintf(stderr, "Unexpected response code!\n");
        return -1;
    }

    return 0;
}
