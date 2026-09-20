// inet_pton va inet_ntop
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

// p: presentation (dang co ., ::)
// n: numberic (dang may tinh hieu duoc)

int main() {
    // inet_pton
    const char* strptr = "192.168.155.2";
    struct in_addr iaddr;
 
    int ret = inet_pton(AF_INET, strptr, (void*)&iaddr);
    if (ret > 0) {
        printf("Address: %d\n", iaddr.s_addr);
    } else if (ret == 0) {
        printf("Wrong format!\n");
    } else if (ret < 0) {
        printf("An error occured!\n");
    }

    // inet_ntop
    char *buf = malloc(INET6_ADDRSTRLEN*sizeof(*buf));
    const char *res = inet_ntop(AF_INET, (const void*)&iaddr, buf, INET_ADDRSTRLEN);

    if (res)
        printf("%s\n", res);
    else
        printf("An error occured!\n");

    free(buf);
    return 0;
}