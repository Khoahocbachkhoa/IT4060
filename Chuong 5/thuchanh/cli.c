#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>

#define PORT 8080
#define MAXLINE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char name[MAXLINE], msg[MAXLINE];
    struct pollfd fds[2];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    // kết nối tới server
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    printf("Enter your name: ");
    fgets(name, MAXLINE, stdin);
    name[strlen(name)-1] = '\0';

    // gửi tên định danh cho server
    if (send(sockfd, name, strlen(name), 0) < 0) {
        perror("Error:");
        exit(EXIT_FAILURE);
    }

    fds[0].fd = fileno(stdin);
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    while (1) {
        if (poll(fds, 2, -1) < 0) {
            perror("Poll error:");
            exit(EXIT_FAILURE);
        }
        // Xử lý stdin
        if (fds[0].revents & POLLIN) {
            int n = read(fds[0].fd, msg, MAXLINE);
            if (n < 0) { /* error */
                perror("read error:");
                exit(EXIT_FAILURE);
            } else if (n == 0) {
                // EOF
            } else {
                msg[n] = '\0';
                // Gửi tin nhắn cho server
                send(sockfd, msg, strlen(msg), 0);
            }
        }
        // Xử lý tin nhắn gửi từ server
        if (fds[1].revents & POLLIN) {
            int n = read(sockfd, msg, MAXLINE);
            if (n < 0) {
                perror("read error");
                exit(EXIT_FAILURE);
            } else if (n == 0) {
                // eof -> server đã ngừng kết nối
                printf("Disconnected to server!\n");
                exit(EXIT_SUCCESS);
            } else {
                msg[n] = '\0';
                printf("%s\n", msg);
            }
        }
    }

    return 0;
}