#include <sys/socket.h>
#include <stdio.h>

int main() {
    // int socket(int family, int type, int protocol)
    // trả về file descriptor nếu thành công, 0 nếu thất bại
    // family: AF_INET (Ipv4 socket)
    // type: SOCK_STREAM (thuong dung cho TCP), SOCK_DGRAM (UDP)
    // protocol thuong duoc de mac dinh la 0

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
        printf("Tao 1 socket TCP thanh cong\n");
    } else {
        printf("That bai!\n");
    }
    // sockfd giống như 1 định danh cho file ứng với việc đọc - ghi socket

    return 0;
}