// Tạo tcp theo dõi nhiều client sử dụng poll()
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERV_PORT 8585
#define MAX_CLIENTS 10

// hàm mô tả xử lý client
void handle_cli(int sockfd);

int main() {
    int listenfd, connfd;
    socklen_t clilen;   
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 10);

    struct pollfd fds[1+MAX_CLIENTS];
    int num_cli = 0;

    //* Y tuong : socket lang nghe cung cho thanh 1 socket trong fds
    /* for (;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen); // fd moi de giao tiep voi client
        
        fds[num_clients].fd = connfd;
        
        fds[num_clients].events = POLLIN | POLLOUT; 

    } */
    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    while (1) {
        int ret = poll(fds, num_cli+1, -1);
        if (ret > 0) {
            // Neu co ket noi toi
            if (fds[0].events & POLLIN) {
                clilen = sizeof(cliaddr);
                // Khoi tao ket noi moi va dua vao fds
                connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
                ++num_cli;
                fds[num_cli].fd = connfd;
                fds[num_cli].events = POLLIN; // * chi can quan tam viec nhan du lieu tu client
                // Viec gui du lieu chung ta chi nen thuc hien khi can thiet, do viec nay chung ta kiem soat duoc
            }
            // Duyet tung ket noi va xu ly can thiet
            for (int i = 1; i <= num_cli; ++i) {
                if (fds[i].revents & POLLIN) {
                    // xu ly viec nhan du lieu tu client!
                }
                // * Xu ly client dong ket noi, ...
                if (fds[i].revents & POLLNVAL) {
                    // 
                    printf("Client dong ket noi!\n");
                }
            }
        }
    }

    return 0;
}