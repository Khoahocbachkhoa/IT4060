#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#define SERV_PORT 8000
#define MAXLINE 1024

void str_cli(FILE *fp, int sockfd);

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("failed to connect to server");
        exit(EXIT_FAILURE);
    }

    str_cli(stdin, sockfd);

    return 0;
}

// Example client using select that handle eof correctly
void str_cli(FILE *fp, int sockfd) {
    int maxfdp1, stdineof, filefd;
    fd_set rset;
    char buf[MAXLINE];
    int n;

    filefd = fileno(fp);
    stdineof = 0;

    for (;;) {
        FD_ZERO(&rset);
        if (stdineof == 0)
            FD_SET(filefd, &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = ((sockfd > filefd) ? sockfd : filefd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        /* if socket is readable */
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = recv(sockfd, buf, MAXLINE, 0)) == 0) {
                if (stdineof == 1)
                    return; /* normal termination */
                else {
                    perror("Server terminated prematurely");
                    exit(EXIT_FAILURE);
                }
            }
            buf[n] = '\0';
            printf("%s\n", buf);
        }

        /* if input is readable */
        if (FD_ISSET(filefd, &rset)) {
            if ((n = read(filefd, buf, MAXLINE)) == 0) {
                stdineof = 1; // reach end of file
                shutdown(sockfd, SHUT_WR);
                FD_CLR(filefd, &rset);
                continue;
            }
            send(sockfd, buf, n, 0);
        }
    }
}
