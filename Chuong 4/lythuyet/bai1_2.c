#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    fork();
    fork();
    fork();
    printf("pid: %d\n", getpid());
    return 0;
}