#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024

int connect_to_server(const char *ip, int port);
int send_all(int fd, const char *buf);
int recv_line(int fd, char *buf, size_t buf_size); // thuc te nen doc tung buffer lon va phan tich ra thanh cac dong
int check_pop3_ok(const char *response);
int send_pop3_command(int sockfd, const char *cmd, char *res, size_t response_size);
int read_multiline_response(int sockfd);
void run_pop3_menu(int sockfd);

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <ip> <port> <username> <password>", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *username = argv[3];
    const char *password = argv[4];

    // connect to server
    int sockfd = connect_to_server(ip, port);
    if (sockfd < 0) {
        fprintf(stderr, "Cannot connect to server\n");
        return 1;
    }

    char res[MAXLINE];
    char cmd[MAXLINE];

    // send username
    snprintf(cmd, sizeof(cmd), "USER %s\r\n", username);
    send_pop3_command(sockfd, cmd, res, MAXLINE);
    // send password
    snprintf(cmd, sizeof(cmd), "PASS %s\r\n", password);
    if (!send_pop3_command(sockfd, cmd, res, MAXLINE)) {
        printf("Wrong password!\n");
        return 1;
    }

    // run menu
    run_pop3_menu(sockfd);
    return 0;
}

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

    if (recv_line(fd, res, sizeof(res)) < 0)
        return -1;

    if (!check_pop3_ok(res))
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

int check_pop3_ok(const char *response) {
    char code[8];
    sscanf(response, "%s", code);
    if (strcmp(code, "+OK") == 0) {
        return 1; // oke
    } else {
        return 0; // error
    }
}

int send_pop3_command(int sockfd, const char *cmd, char *res, size_t response_size) {
    // Gui command
    send_all(sockfd, cmd);
    // Doc phan hoi
    recv_line(sockfd, res, response_size);
    // In ra phan hoi ngan gon
    //printf("%s\n", res);
    // Kiem tra ma phan hoi
    return check_pop3_ok(res);
}

int read_multiline_response(int sockfd) {
    char line[MAXLINE];
    int ret;
    while (((ret = recv_line(sockfd, line, MAXLINE)) >= 0) && strcmp(line, ".") != 0) {
        printf("%s\n", line);
    }

    return ret;
}

void run_pop3_menu(int sockfd) {
    printf("==== POP3 CLIENT ====\n");
    printf("1. Show mailbox status\n");
    printf("2. List messages\n");
    printf("3. Retrieve a message\n");
    printf("4. Quit\n");

    int choice;
    char line[MAXLINE];
    char res[MAXLINE];
    while (1) {
        scanf("%d", &choice);
        switch (choice)
        {
            case 1:
                if (send_pop3_command(sockfd, "STAT\r\n", res, MAXLINE))
                    printf("%s\n", res);
                break;

            case 2:
                if (send_pop3_command(sockfd, "LIST\r\n", res, MAXLINE))
                    read_multiline_response(sockfd);
                break;
            
            case 3:
                printf("Enter which message: ");
                int n;
                scanf("%d", &n);
                snprintf(line, MAXLINE, "RETR %d\r\n", n);
                if (send_pop3_command(sockfd, line, res, MAXLINE))
                    read_multiline_response(sockfd);
                break;

            case 4:
                send_pop3_command(sockfd, "QUIT\r\n", res, MAXLINE);
                return;

            default:
                break;
        }
    }
}