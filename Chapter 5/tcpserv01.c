#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

#define SERV_PORT 8000
#define MAXLINE 1024
#define LISTENQ 10

void modify(char *buf, size_t sz) {
    for (int i = 0; i < sz; ++i) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] += 'A' - 'a';
        }
    }
}

//* note: this server cannot handle DOS attack
int main() {
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
    printf("Server is listening in port: %d\n", SERV_PORT);

    maxfd = listenfd; /* initialize */
    maxi = -1; /* index of client[] */
    for (i = 0; i < FD_SETSIZE; ++i)
        client[i] = -1;

    /* allset : all descriptor need to track */
    /* assign rset to allset so we dont need to reset the allset when call select() */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;) {
        // Làm như vậy sẽ tránh phải khởi tạo lại, chỉ cần copy
        rset = allset; /* structure assignment */
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL); //* note: when call select, allset is not changed

        /* if a new client connected */
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

            for (i = 0; i < FD_SETSIZE; ++i) {
                /* if there is an available slot */
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                printf("Too many clients!\n");
                exit(EXIT_FAILURE);
            }

            //* Mỗi khi có client kết nối thì thêm vào allset, và trong loop thì copy rset với allset
            FD_SET(connfd, &allset); // add new descriptor to set
            if (connfd > maxfd) maxfd = connfd;
            if (i > maxi) maxi = i; /* max index in client[] */
            if (--nready <= 0)
                continue; /* no more readable descriptor */
        }

        /* check all clients for data */
        for (i = 0; i <= maxi; ++i) {
            if ((sockfd = client[i]) < 0) /* if this client is disconnected! */
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = recv(sockfd, buf, MAXLINE, 0)) == 0) { /* means client close connect */
                    close(sockfd);
                    // Khi một client ngắt kết nối thì clear nó đi
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    buf[n] = '\0';
                    printf("Received: %s\n", buf);
                    /* modify to uppercase and resend to client */
                    modify(buf, n);
                    send(sockfd, buf, n, 0);
                }
            }

            //* do this so dont need to check all clients
            if (--nready <= 0)
                continue; /* no more readable descriptor */
        }
    }

    return 0;
}