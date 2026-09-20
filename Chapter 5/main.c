#include <stdio.h>

int main() {
    FILE *fp = stdin;
    int fd = fileno(fp);
    printf("%d\n", fd);
    return 0;
}