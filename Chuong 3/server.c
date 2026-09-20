#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "sock_ntop.h"

#define PORT 5349

int main() {
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    int cli_addr_len = sizeof(cli_addr);
    int sockfd, newsockfd;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error:");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    // bind với tất cả interface
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error:");
        exit(0);
    }

    if (listen(sockfd, 3) < 0) {
        perror("Error:");
        exit(0);
    }

    printf("Server đang đợi kết nối!\n");

    if ((newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_addr_len)) >= 0) {
        printf("Chấp nhận kết nối từ : %s\n", sock_ntop((struct sockaddr*)&cli_addr, cli_addr_len));
    } else {
        perror("Loi!");
        exit(0);
    }

    // Đọc thông điệp từ client
    char buffer[128];
    read(newsockfd, buffer, sizeof(buffer));
    printf("Nhận thông điệp từ client: %s\n", buffer);

    // Gửi lại thông điệp cho client
    char response[128] = "Hello client!";
    write(newsockfd, response, sizeof(response));
    printf("Đã gửi lại thông điệp cho client!\n");

    return 0;
} 