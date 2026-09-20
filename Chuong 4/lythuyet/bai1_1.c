#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Tiến trình con, pid = %d\n", getpid());
    } else if (pid > 0) {
        printf("Tiến trình cha, pid = %d có con pid = %d\n", getpid(), pid);
    } else {
        perror("fork thất bại");
    }

    return 0;
}