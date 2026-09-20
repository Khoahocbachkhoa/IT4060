#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "http.h"

#define PORT 8080
#define LISTENQ 100
#define MAXLINE 1024

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("Child %d terminated!\n", pid);
    return; 
}

int main(int argc, char const *argv[]) {
    pid_t pid;
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    printf("Http server is running in port %d\n", PORT);

    signal(SIGCHLD, sig_chld);
    for (;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

        pid = fork();

        if (pid == 0) { /* child process */
            close(listenfd);
            handle_request(connfd);
            close(connfd);
            exit(0);
        }

        close(connfd);
    }
    
    return 0;
}