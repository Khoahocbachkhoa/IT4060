#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#define PORT 8080
#define MAXLINE 1024
#define KEY "a&8da#$@fsdhjk"

void xor_cipher(char *data, int len, char *key);
int sock_cmp(struct sockaddr_in a, struct sockaddr_in b);

int main(int argc, char const *argv[]) {
    int sockfd;
    char buf[MAXLINE];
    struct sockaddr_in servaddr, recv_servaddr;
    fd_set rset;
    struct timeval tv;

    // creating socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket error!");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    ssize_t n;
    socklen_t len = sizeof(recv_servaddr);

    // send hello to server to etablish connection
    snprintf(buf, MAXLINE, "Hello");
    printf("To server: %s\n", buf);
    xor_cipher(buf, strlen(buf), KEY);
    sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    if (select(sockfd+1, &rset, NULL, NULL, &tv) == 0) {
        perror("Timeout has expired!\n");
        exit(EXIT_FAILURE);
    }
    // recv response from server
    if ((n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&recv_servaddr, &len)) == -1) {
        perror("read error");
        exit(EXIT_FAILURE);
    }
    buf[n] = '\0';

    if (!sock_cmp(servaddr, recv_servaddr)) {
        perror("Received from unexpected server!");
        exit(EXIT_FAILURE);
    }

    xor_cipher(buf, n, KEY);
    printf("Server : %s\n", buf);

    // send response to server
    printf("Enter your message: ");
    fgets(buf, MAXLINE, stdin);
    buf[strlen(buf)-1] = '\0'; // remove '\n'
    //printf(buf, MAXLINE, "Hello server, have a nice day!");
    printf("To server: %s\n", buf);
    xor_cipher(buf, strlen(buf), KEY);
    sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);

    if (select(sockfd+1, &rset, NULL, NULL, &tv) == 0) {
        perror("Timeout has expired!\n");
        exit(EXIT_FAILURE);
    }

    // recv acknowledgment from server
    if ((n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&servaddr, &len)) == -1) {
        perror("read error");
        exit(EXIT_FAILURE);
    }
    buf[n] = '\0';

    if (!sock_cmp(servaddr, recv_servaddr)) {
        perror("Received from unexpected server!");
        exit(EXIT_FAILURE);
    }

    xor_cipher(buf, n, KEY);
    printf("Server : %s\n", buf);

    return 0;
}

void xor_cipher(char *data, int len, char *key) {
    int key_len = strlen(key);
    for (int i = 0; i < len; ++i) {
        data[i] ^= key[i % key_len];
    }
}
int sock_cmp(struct sockaddr_in servaddr, struct sockaddr_in recv_servaddr) {
    return memcmp(&servaddr.sin_addr, &recv_servaddr.sin_addr, sizeof(servaddr.sin_addr)) == 0 &&
        memcmp(&servaddr.sin_port, &recv_servaddr.sin_port, sizeof(servaddr.sin_port)) == 0 &&
        memcmp(&servaddr.sin_family, &recv_servaddr.sin_family, sizeof(servaddr.sin_family)) == 0;
}