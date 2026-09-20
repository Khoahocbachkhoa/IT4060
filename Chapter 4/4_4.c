// int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
// Gán địa chỉ nội bộ với 1 socket
// server cần làm việc này vì cấn 1 địa chỉ cụ thể thay vì client

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
/*
    * THông thường"
    * Client tạo socket, không cần bind, kernel dựa trên địa chỉ đích sẽ tự bind tới card mạng tương ứng
    * Server cần bind tới card cụ thể để client biết gửi đi đâu
*/

int main() {
    
}