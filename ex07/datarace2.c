#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int count;

void *inc_count(void *arg) {
    pthread_mutex_lock(&lock);
    ++count;
    pthread_mutex_unlock(&lock);
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, inc_count, NULL);
    inc_count(NULL);
    pthread_join(tid, NULL);
    return (0);
}
