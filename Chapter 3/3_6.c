#include <stdio.h>
#include <arpa/inet.h>

int main() {
    // inet_aton : chuyen doi dia chi ipv4 dang . sang so nguyen 32 bit
    const char *ipaddr = "192.168.255.1";
    struct in_addr ans;

    int ret = inet_aton(ipaddr, &ans);
    if (ret == 1) {
        printf("Address : %d\n", ans.s_addr);
    } else {
        printf("Invalid format\n");
    }

    // inet_addr: tuong tu, tra ve truc tiep so nguyen 32 bit
    in_addr_t add = inet_addr(ipaddr);
    if (add != INADDR_NONE) {
        printf("Address: %d\n", add);
    } else {
        printf("Invalid format\n");
    }

    // inet_ntoa: chuyen doi so nguyen 32 bit dang dia chi thap phan co .
    struct in_addr addr;
    addr.s_addr = inet_addr("10.10.127.125");

    char *res = inet_ntoa(addr);
    if (res)
        printf("Address: %s\n", res);
    else
        printf("Error!\n");

    return 0;
}