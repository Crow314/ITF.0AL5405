#include <pthread.h>

int count;

void *inc_count(void *arg) {
    ++count;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, inc_count, NULL);
    inc_count(NULL);
    pthread_join(tid, NULL);
    return (0);
}
