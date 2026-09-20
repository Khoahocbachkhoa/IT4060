#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

int main() {
    printf("Max size of fd_set: %d\n", FD_SETSIZE);
    return 0;
    fd_set fdset;
    // Đặt tất cả các bit về 0
    FD_ZERO(&fdset);

    FD_SET(3, &fdset); // set bit thu 3
    FD_SET(4, &fdset); // set bit thu 4

    long ival = fdset.__fds_bits[0];
    printf("fd_bits[0] = %ld\n", ival);
    for (int i = sizeof(ival)*8-1; i >= 0; --i) {
        printf("%d", (ival >> i) & 1);
    }
    putchar('\n');

    return 0;
}