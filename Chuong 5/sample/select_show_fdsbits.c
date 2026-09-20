// select_show_fdsbits.c
// Example to inspect fd_set.fds_bits[] on macOS after setting STDIN_FILENO.

#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

int main(void) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);  // STDIN_FILENO = 0

    printf("FD_SETSIZE = %d\n", FD_SETSIZE);
    printf("STDIN_FILENO = %d\n", STDIN_FILENO);

    // Each element in fds_bits is typically a long (64-bit on macOS)
    // We print them in binary to visualize which bit got set
    printf("\nContents of fds_bits[] after FD_SET(STDIN_FILENO):\n");

    size_t n_bits = sizeof(readfds.fds_bits) / sizeof(readfds.fds_bits[0]);
    for (size_t i = 0; i < n_bits; i++) {
        printf("fds_bits[%zu] = 0x%016lx  (binary: ", i, readfds.fds_bits[i]);
        for (int b = sizeof(long) * 8 - 1; b >= 0; b--) {
            putchar((readfds.fds_bits[i] & (1UL << b)) ? '1' : '0');
        }
        printf(")\n");
    }

    printf("\nWaiting for input on stdin using select()...\n");
    printf("(Type something and press Enter)\n\n");

    struct timeval tv = { .tv_sec = 10, .tv_usec = 0 }; // 10s timeout
    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ret < 0) {
        perror("select");
        return 1;
    } else if (ret == 0) {
        printf("Timeout reached: no input detected.\n");
    } else if (FD_ISSET(STDIN_FILENO, &readfds)) {
        printf("STDIN is ready to read!\n");
        char buf[256];
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("You entered: %s\n", buf);
        }
    }

    return 0;
}
