#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#include "pop3_module.h"
#include "socket_utils.h"

int recv_line(int fd, char *buf, size_t buf_size) {
    if (buf == NULL || buf_size == 0)
        return -1;

    size_t pos = 0;
    while (pos < buf_size - 1) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);

        if (n == 0) { /* server disconnected */
            break;
        }

        if (n < 0) { /* Read error */
            return -1;
        }

        buf[pos++] = c;

        if (pos >= 2 && buf[pos - 2] == '\r' && buf[pos - 1] == '\n') {
            buf[pos-2] = '\0';
            return (int)pos;
        }
    }

    buf[pos] = '\0';

    // Đọc quá buf_size
    return -1;
}

int read_multiline_response(int sockfd) {
    char line[1024];
    int ret;
    while (((ret = recv_line(sockfd, line, 1024)) >= 0) && strcmp(line, ".") != 0) {
        printf("%s\n", line);
    }

    return ret;
}


int pop3_find_email_by_subject(const char * server_ip, int server_port, const char *username, const char *password) {
    int sockfd;
    if ((sockfd = connect_to_server(server_ip, server_port)) < 0) {
        fprintf(stderr, "Pop3: cannot connect to server!\n");
        return -1;
    }

    char req[256];
    char res[256];
    recv_line(sockfd, res, 256);

    snprintf(req, sizeof(req), "USER %s\r\n", username);
    send_cmd(sockfd, req);
    recv_line(sockfd, res, 256);

    snprintf(req, sizeof(req), "PASS %s\r\n", password);
    send_cmd(sockfd, req);
    recv_line(sockfd, res, 256);

    // Doc email client vua gui
    snprintf(req, sizeof(req), "RETR 1\r\n");
    send_cmd(sockfd, req);
    recv_line(sockfd, res, 256);

    read_multiline_response(sockfd);

    // Gui QUIT
    snprintf(req, sizeof(req), "QUIT\r\n");
    send_cmd(sockfd, req);
    return 0;
}