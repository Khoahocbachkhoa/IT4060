#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8080

//? ví dụ: tạo 1 socket và kết nối tới 127.0.0.1:8080
int main() {
    struct sockaddr_in serv_addr;

    // Khởi tạo file descriptor của socket
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("An error occured!\n");
        exit(0);
    }

    // Khởi tạo địa chỉ cho server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // htons() dùng để chuyển đổi thứ tự byte sang network order thay vì host order
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Error!\n");
        exit(0);
    }

    // dùng hàm connet để kết nối tới server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Co loi khi ket noi\n");
        exit(0);
    }

    printf("Hoan tat thiet lap ket noi toi server!");
    return 0;
}


// int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen)
    // * dùng bởi client đề thiết lập kết nối tới server
    // * client ko cần bind socket?
    // * tra ve 0 neu thanh cong va cac so am tuy loai loi (unreachable, refuse connet, timeout...)