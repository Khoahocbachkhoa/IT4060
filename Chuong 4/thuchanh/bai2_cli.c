#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 512
#define PORT 8111

void handle(int sockfd) {
    char buf[MAXLINE], ans[8];

    read(sockfd, buf, MAXLINE);
    printf("%s\n", buf);
    
    while(read(sockfd, buf, MAXLINE) > 0) {
        printf("%s\n", buf);
        fgets(ans, 8, stdin);
        ans[1] = '\0';
        send(sockfd, ans, 2, 0);
        read(sockfd, buf, MAXLINE);
        printf("%s\n", buf);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    handle(sockfd);

    return 0;
}