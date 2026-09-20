#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>

int main() {
    // getsockopt() and setsockopt() : modify socket option in os-level
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // SO_REUSEADDR: enable reuse in an time-wait socket
    // if turn off this option, there may be "sock already in use"
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    // SO_RECVBUF: set recv buffer
    int rcvbuf = 65536; // 64 KB
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    // setimeout
    struct timeval timeout;
    timeout.tv_sec = 5; // 5s
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    // enable TCP_NODELAY - disable NAGLE alg
    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    // getsockopt : get the attribute of option we set
    
    // get buffer size of socket
    int bufsize;
    socklen_t len = sizeof(bufsize);
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufsize, &len);
    printf("Bufsize : %d\n", bufsize);
    // get timeout
    len = sizeof(timeout);
    getsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, &len);
    printf("Time out: %ld\n", timeout.tv_sec);

    return 0;
}
