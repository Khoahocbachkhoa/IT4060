#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* foo(void* args) {
    printf("Hello world!\n");
    return NULL;
}

int main() {
    // Tách luồng
    pthread_t tid;
    pthread_create(&tid, NULL, foo, NULL);
    pthread_detach(tid); // tách luồng chạy nền và tự giải phóng khi chạy xong, ko cần gọi join()
    sleep(5);
    return 0;
}