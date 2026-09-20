#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>

int main() {
    fd_set read_fds;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    FD_ZERO(&read_fds);
    // them sockfd vao tap can theo doi
    FD_SET(sockfd, &read_fds);
    // ...
    if (FD_ISSET(sockfd, &read_fds)) {
        printf("Socket dang duoc theo doi!\n");
    } else {
        printf("Socket khong duoc theo doi!\n");
    }
    // Bỏ theo dõi sockfd
    FD_CLR(sockfd, &read_fds);

    return 0;
}