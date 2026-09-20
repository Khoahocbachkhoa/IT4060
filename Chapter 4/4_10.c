#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

// listen(int sockfd, int backlog): nghe va chap nhan toi da backlog ket noi!

// int getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen);
// int getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen);

// * Trả về địa chỉ của giao thức phía bên máy chủ và client dựa vào sockfd
// * tham số addrlen dùng cả làm giá trị và trả về

int main() {
    
}