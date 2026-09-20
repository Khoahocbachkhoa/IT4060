#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() {
    char str[32];
    printf("Nhập địa chỉ ipv4: ");
    fgets(str, sizeof(str), stdin);
    // Bo '\n'
    str[strlen(str)-1] = '\0';

    // printf("%s\n", str);

    struct in_addr iaddr;

    // Chuyển dạng chuỗi -> nhị phân
    if (inet_pton(AF_INET, str, &iaddr) <= 0) {
        perror("Lỗi:");
        exit(0);
    } else {
        printf("Địa chỉ dạng nhị phân : %u\n", ntohl(iaddr.s_addr));
    }

    // Chuyển dạng nhị phân -> chuỗi
    char *buf = malloc(INET6_ADDRSTRLEN*sizeof(char));
    if (inet_ntop(AF_INET, &iaddr, buf, INET6_ADDRSTRLEN) != NULL)
        printf("Địa chỉ dạng chuỗi: %s\n", buf);
    else {
        perror("Lỗi:");
        exit(0);
    }

    return 0;

}