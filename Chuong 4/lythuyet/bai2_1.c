#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* foo(void* args) {
    printf("Hello from thread!\n");
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, foo, NULL);
    pthread_join(tid, NULL);

    return 0;
}