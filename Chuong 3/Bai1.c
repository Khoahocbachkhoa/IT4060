#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080

int main() {
    struct sockaddr_in sin;

    const char *strptr = "192.168.1.1";

    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, strptr, &sin.sin_addr) < 0) {
        perror("Loi chuyen doi:");
        return -1;
    }

    printf("Khoi tao thanh cong!\n");
    return 0;
}