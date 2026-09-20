#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define LISTEN_Q 10
#define MAXLINE 1024

void handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int fd) {
    // Đọc email,password gửi từ client 
    char buf[MAXLINE];
    char email[32];
    char password[100];
    
    int n = recv(fd, buf, sizeof(buf), 0);
    buf[n] = '\0';

    // 1 dòng : email,pass
    char *comma = strchr(buf, ',');

    if (comma != NULL) {
        int len = comma - buf;

        // validate email len
        if (len >= sizeof(email))
        {
            printf("Email too long!");
            snprintf(buf, sizeof(buf), "Err email quá dài");
            send(fd, buf, strlen(buf), 0);
            return;
        }

        // copy email
        strncpy(email, buf, len);
        email[len] = '\0';

        // validate password len
        if (strlen(comma + 1) >= sizeof(password))
        {
            printf("Password too long!");
            snprintf(buf, sizeof(buf), "Err mật khẩu quá dài");
            send(fd, buf, strlen(buf), 0);
            return;
        }

        // copy password
        strcpy(password, comma + 1);
    } else {
        // Invalid format
        snprintf(buf, sizeof(buf), "Err cú pháp không hợp lệ");
        send(fd, buf, strlen(buf), 0);
        return;
    }

    //
    printf("Email : %s\n", email);
    printf("Pass: %s\n", password);

    // Lưu vào file luôn
    // Gia su luu thanh cong vao file (tranh xu ly viec khong lien quan lap trinh mang)
    snprintf(buf, sizeof(buf), "OK đăng ký thành công");
    send(fd, buf, strlen(buf), 0);
}

int main(int argc, char const *argv[])
{
    pid_t pid;
    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    // tạo socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // Khởi tạo địa chỉ của server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind socket voi dia chi server
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Lắng nghe trên socket vừa tạo
    if (listen(sockfd, LISTEN_Q) < 0) 
    {
        perror("Listen error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Gọi handler mỗi khi có tiến trình con kết thúc và gửi signal cho tiến trình cha
    signal(SIGCHLD, handler);

    // Chấp nhận kết nối từ client
    while (1) 
    {
        // Để đơn giản, tạo luồng con xử lý client
        clilen = sizeof(cli_addr);
        connfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (connfd < 0)
        {
            perror("Accept error");
            continue;
        }

        pid = fork();
        if (pid == 0)
        {
            close(sockfd);
            // Xử lý cho từng client
            handle_client(connfd);
            // Kết thúc tiến trình con
            // * quan trọng
            close(connfd);
            exit(EXIT_SUCCESS);
        }

        close(connfd);
    }

    return 0;
}