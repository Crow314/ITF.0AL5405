#include <pthread.h>

pthread_mutex_t m;
pthread_mutexattr_t attr;

main()
{
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m, &attr);

    pthread_mutex_lock(&m);
    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    pthread_mutex_unlock(&m);
}
