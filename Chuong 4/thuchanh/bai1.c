#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid;
    pid = fork();

    if (pid == 0) {
        printf("Child procees, pid = %d\n", getpid());
    } else {
        printf("Parent process, pid = %d\n", getpid());
        sleep(30);
    }

    return 0;
}