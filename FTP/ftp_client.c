#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 1024

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
    printf("Receive response: %s", buf);
    return n;
}

int connect_to_ftpserver() {
    char buf[MAXLINE];
    struct sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(21);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error!");
        // Trả về 1 fd ko hợp lệ nếu có lỗi kết nối
        return -1; 
    }

    // Nhận thông báo chào mừng từ server
    recv_response(sockfd, buf, MAXLINE);

    return sockfd;
}

int ftp_login(int sockfd, const char *username, const char *password) {
    char buf[MAXLINE];
    int code;

    snprintf(buf, MAXLINE, "USER %s\r\n", username);
    send_cmd(sockfd, buf);
    recv_response(sockfd, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 331)
        return 1;

    snprintf(buf, MAXLINE, "PASS %s\r\n", password);
    send_cmd(sockfd, buf);
    recv_response(sockfd, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 230)
        return 1;

    // trả về 0 nếu đăng nhập thành công
    return 0;
}

int ftp_pasv(int sockfd) {
    char buf[MAXLINE];
    int code;

    snprintf(buf, MAXLINE, "PASV\r\n");
    send_cmd(sockfd, buf);
    recv_response(sockfd, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 227) {
        printf("PASV error\n");
        return -1;
    }

    // Parse IP and port from response
    char ip[32];
    int port, h1, h2, h3, h4, p1, p2;
    if (sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        return -1;
    }
    
    port = p1 * 256 + p2;
    snprintf(ip, 32, "%d.%d.%d.%d", h1, h2, h3, h4);

    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;

    memset(&data_addr, 0, sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &data_addr.sin_addr);

    if (connect(connfd, (struct sockaddr*)&data_addr, sizeof(data_addr)) < 0) {
        perror("connect error");
        return -1;
    }

    return connfd;
}

int ftp_list(int sockfd) {
    int connfd;
    char buf[MAXLINE];
    int code;

    if ((connfd = ftp_pasv(sockfd)) < 0)
        return 1;
    
    snprintf(buf, MAXLINE, "LIST\r\n");
    send_cmd(sockfd, buf);
    recv_response(sockfd, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 150) {
        return 1;
    }

    // Lấy danh sách file qua data socket
    int n;
    while ((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(connfd);
    recv_response(sockfd, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 226)
        return 1;

    return 0;
}

int ftp_retr(int sockfd, const char *filename) {
    int connfd, code; // data socket descriptor
    char buf[MAXLINE];

    // Open data socket
    if ((connfd = ftp_pasv(sockfd)) < 0) {
        printf("Failed open data socket!\n");
        return -1;
    }
    // Gui retr qua control socket
    snprintf(buf, MAXLINE, "RETR %s\r\n", filename);
    send_cmd(sockfd, buf);

    // Nhan phan hoi va phan tich ma tra ve
    recv_response(sockfd, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 150) {
        printf("Loi khi tai file!\n");
        return -1;
    }
    // Mo file cuc bo de ghi!
    int filefd = open(filename, O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
    if (filefd == -1) {
        printf("Open file error!\n");
        return -1; // open file error 
    }
    // Tai ve qua data socket va ghi ra filefd
    int n;
    while ((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
        write(filefd, buf, n);
    }

    close(connfd);
    close(filefd);

    recv_response(sockfd, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 226) {
        printf("Loi khi tai file!\n");
        return -1;
    }
    return 1;
}

int ftp_stor(int sockfd, const char *filename) {
    int connfd, code;
    char buf[MAXLINE];

    // open data socket
    if ((connfd = ftp_pasv(sockfd)) < 0)
        return -1;

    snprintf(buf, MAXLINE, "STOR %s\r\n", filename);
    send_cmd(sockfd, buf);
    recv_response(sockfd, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 150)
        return -1;

    // Mo file
    int filefd = open(filename, O_RDONLY);
    if (filefd == 1)
        return -1;
    // Gui file qua data socket
    int n;
    while ((n = recv(filefd, buf, MAXLINE, 0)) > 0) {
        send(connfd, buf, n, 0);
    }

    close(connfd);
    close(filefd);

    recv_response(sockfd, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 226)
        return -1;
    return 0;
}

int ftp_delete(int sockfd, const char *filename) {
    int code;
    char buf[MAXLINE];

    snprintf(buf, MAXLINE, "DELE %s\r\n", filename);
    send_cmd(sockfd, buf);

    recv_response(sockfd, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 250)
        return -1;
    return 0;
}

void ftp_cwd(int sockfd, const char *dirname) {
    char buf[MAXLINE];

    snprintf(buf, MAXLINE, "CWD %s\r\n", dirname);
    send_cmd(sockfd, buf);

    recv_response(sockfd, buf, MAXLINE);
}

int main(int argc, char *argv[])
{
    int sockfd;
    const char *username = "user", *password = "pass";
    char cmd[8], filename[32], dirname[32];

    if ((sockfd = connect_to_ftpserver()) < 0)
        return -1;
    
    if (ftp_login(sockfd, username, password) != 0)
        return -1;

    while (1) {
        scanf("%s", cmd);
        if (strcmp(cmd, "list") == 0) {
            ftp_list(sockfd);
            getchar();
        } else if (strcmp(cmd, "get") == 0) {
            scanf("%s", filename);
            getchar();
            ftp_retr(sockfd, filename);
        } else if (strcmp(cmd, "put") == 0) {
            scanf("%s", filename);
            getchar();
            ftp_stor(sockfd, filename);
        } else if (strcmp(cmd, "delete") == 0) {
            scanf("%s", filename);
            getchar();
            ftp_delete(sockfd, filename);
        } else if (strcmp(cmd, "cd") == 0) {
            scanf("%s", dirname);
            getchar();
            ftp_cwd(sockfd, dirname);
        } else if (strcmp(cmd, "quit") == 0) {
            close(sockfd);
            break;
        } 
    }

    return EXIT_SUCCESS;
}
