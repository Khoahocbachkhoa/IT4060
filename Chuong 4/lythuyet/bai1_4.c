#include <stdio.h>
#include <unistd.h>

int main() {
    if (fork() == 0) {
        sleep(10);
        printf("Tien trinh con ket thuc!\n");
        _exit(0);
    } else {
        wait(NULL); // Đợi 1 tiến trình con kết thúc?
        printf("Tien trinh cha ket thuc!\n");
    }

    return 0;
}