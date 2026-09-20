#include <stdio.h>
#include <pthread.h>

void* print(void* args) {
    int* n = (int*)args;
    printf("%d ", *n);
    return NULL;
}

int main() {
    pthread_t tids[10];
    int nums[10];

    for (int i = 0; i != 10; ++i) {
        nums[i] = i + 1;
        pthread_create(&tids[i], NULL, print, &nums[i]);
    }

    // Doi cac tien trinh con ket thuc
    for (int i = 0; i != 10; ++i) {
        pthread_join(tids[i], NULL);
    }
    putchar('\n');
    return 0;
}