#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/poll.h>

#define PORT 8080
#define MAXLINE 1024
#define MAXSIZE 100

int main() {
    int listenfd, connfd;
    socklen_t clilen;   
    struct sockaddr_in cliaddr, servaddr;
    struct pollfd fds[MAXSIZE];
    char names[MAXSIZE][MAXLINE];
    char buf[MAXLINE], msg[MAXLINE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 10);
    
    int sz = 1;
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    for (int i = 1; i < MAXSIZE; ++i) {
        names[i][0] = '\0';
    }

    while (1) {
        if (poll(fds, sz, -1) < 0) {
            perror("poll error");
            exit(EXIT_FAILURE);
        }
        // Nếu có client kết nối tới
        if (fds[0].revents & POLLIN) {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

            if (sz < MAXSIZE) {
                fds[sz].fd = connfd;
                fds[sz].events = POLLIN;
                ++sz;
            } else {
                // Từ chối nếu số lượng kết nối đã đầy
                close(connfd);
            }
        }
        // Kiểm tra tin nhắn gửi từ các client
        for (int i = 1; i < sz; ++i) {
            if (fds[i].revents & POLLIN) {
                int n = read(fds[i].fd, buf, MAXLINE);
                if (n < 0) {
                    perror("read error");
                    exit(EXIT_FAILURE);
                } else if (n == 0) {
                    // eof -> client đã disconnect
                    close(fds[i].fd);
                    printf("%s disconnected!\n", names[i]);
                    // Dồn mảng
                    fds[i] = fds[sz-1];
                    strcpy(names[i], names[sz-1]);
                    --sz;
                    --i; // Ko skip client cuối vừa chèn vào
                    continue;
                } else {
                    buf[n] = '\0';
                }

                // Nếu vẫn chưa có tên -> nhận tên từ client
                if (names[i][0] == '\0') {
                    strcpy(names[i], buf);
                    printf("%s connected!\n", names[i]);
                    continue;
                }
                // Broadcast tin nhắn tới các client khác
                snprintf(msg, MAXLINE, "%s: %s", names[i], buf);
                for (int j = 1; j < sz; ++j) {
                    if (j != i) {
                        if (send(fds[j].fd, msg, strlen(msg), 0) < 0) {
                            perror("send error");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
    }

    return 0;
}