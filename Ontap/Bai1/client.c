#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#define PORT 8080
#define MAXLINE 1024

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect error!");
        exit(EXIT_FAILURE);
    }

    char buf[MAXLINE];

    fgets(buf, sizeof(buf), stdin);
    send(sockfd, buf, strlen(buf), 0);

    // Đợi phản hồi từ server
    int n = recv(sockfd, buf, sizeof(buf), 0);
    buf[n] = '\0';

    printf("%s\n", buf);
    return 0;
}