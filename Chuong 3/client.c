#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "sock_ntop.h"

#define PORT 5349

int main() {
    struct sockaddr_in serv_addr;
    int serv_addr_len = sizeof(serv_addr);
    int sockfd;
    char str[32];

    // 127.0.0.1
    printf("Nhập địa chỉ của server: ");
    fgets(str, sizeof(str), stdin);
    str[strlen(str)-1] = '\0';

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, str, &serv_addr.sin_addr) <= 0) {
        perror("Error:");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error:");
        exit(0);
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0) {
        printf("Kết nối tới server: %s!\n", sock_ntop((struct sockaddr*)&serv_addr, serv_addr_len));
    } else {
        perror("Error:");
        exit(0);
    }

    // Gửi thông điệp tới server
    char message[128] = "Hello server";
    write(sockfd, message, strlen(message));

    // Đọc thông điệp từ client
    char buffer[128];
    read(sockfd, buffer, sizeof(buffer));
    printf("Nhận thông điệp từ server: %s\n", buffer);

    return 0;
}