#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#define MAXLINE 1024
#define OPENMAX 100
#define PORT 8080
#define MAXQ 30

int main(int argc, char *argv[])
{
    int i, maxi, listenfd, connfd, sockfd;
    int nready;
    ssize_t n;
    socklen_t clilen;
    char buf[MAXLINE];
    struct pollfd client[OPENMAX];
    struct sockaddr_in cli_addr, serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, MAXQ) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    client[0].fd = listenfd;
    client[0].events = POLLIN;
    for (int i = 1; i < OPENMAX; ++i)
        client[i].fd = -1; // Indicate this is an available entry
    maxi = 0; // max index of client array
    
    for (;;)  {
        nready = poll(client, maxi + 1, -1);

        if (client[0].revents & POLLIN) { // If new client connect
            clilen = sizeof(cli_addr);
            connfd = accept(listenfd, (struct sockaddr*)&serv_addr, &clilen);
            // find first available slot for new client
            for (int i = 1; i < OPENMAX; ++i) {
                if (client[i].fd == -1) {
                    client[i].fd = connfd;
                    client[i].events = POLLIN;
                    break;
                }
            }
            if (i == OPENMAX) {
                perror("Too many cliens connected!");
                exit(EXIT_FAILURE);
            }
            if (i > maxi)
                maxi = i;
            if (--nready <= 0)
                continue; // no more readable socket
        }

        // Check for client's socket
        for (int i = 1; i <= maxi; ++i) {
            if ((sockfd = client[i].fd) == -1)
                continue;
            // check for POLLER
            if (client[i].revents & POLLERR) {
                // fetch error
                int err;
                socklen_t errlen = sizeof(err);

                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen);
                errno = err;
                perror("POLLERR");

                close(sockfd);
                client[i].fd = -1;
                continue;
            }
            // check for data
            if (client[i].revents & POLLIN) {
                if ((n = read(sockfd, buf, MAXLINE)) < 0) {
                    perror("read error");
                } else if (n == 0) {
                    // mean client close connection
                    close(sockfd);
                    client[i].fd = -1;
                } else {
                    buf[n] = '\0';
                    printf("%s\n", buf);
                }
            }
            if (--nready <= 0)
                continue;
        }
    }
    
    return EXIT_SUCCESS;
}
