#include <pthread.h>
#include <stdio.h>

#define QSIZE 10

typedef struct {
    pthread_mutex_t buf_lock;
    int start;
    int num_full;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    int data[QSIZE]; 
} circ_buf_t;

void put_cb_data(circ_buf_t *cbp, int data) {
    pthread_mutex_lock(&cbp->buf_lock);

    /* wait while the buffer is full */
    while (cbp->num_full == QSIZE)
        pthread_cond_wait(&cbp->notfull, &cbp->buf_lock);
    
    cbp->data[(cbp->start + cbp->num_full) % QSIZE] = data;
    cbp->num_full++;

    // Debug
    printf("%p: Enqueue %d\n", pthread_self(), data);
    
    /* let a waiting reader know there’s data */
    pthread_cond_signal(&cbp->notempty);
    pthread_mutex_unlock(&cbp->buf_lock);
}

int get_cb_data(circ_buf_t *cbp) {
    int data;
    
    pthread_mutex_lock(&cbp->buf_lock);

    /* wait while there’s nothing in the buffer */
    while (cbp->num_full == 0)
        pthread_cond_wait(&cbp->notempty, &cbp->buf_lock);

    data = cbp->data[cbp->start];
    cbp->start = (cbp->start + 1) % QSIZE;
    cbp->num_full--;

    // Debug
    printf("%p: Dequeue %d\n", pthread_self(), data);

    /* let a waiting writer know there’s room */
    pthread_cond_signal(&cbp->notfull);
    pthread_mutex_unlock(&cbp->buf_lock);

    return (data);
}

// Threads

void *enqueue30(void *arg) {
    circ_buf_t *cbp = (circ_buf_t*) arg;

    for (int i=1; i<=30; i++) {
        put_cb_data(cbp, i);
    }
}

void *dequeue10(void *arg) {
    circ_buf_t *cbp = (circ_buf_t*) arg;

    for (int i=0; i<10; i++) {
        get_cb_data(cbp);
    }
}

int main(int argc, char **argv) {
    pthread_t t_e1, t_e2, t_d1, t_d2, t_d3, t_d4, t_d5, t_d6;
    circ_buf_t cbp;

    cbp.start = 0;
    cbp.num_full = 0;

    pthread_mutex_init(&(cbp.buf_lock), NULL);
    pthread_cond_init(&(cbp.notfull), NULL);
    pthread_cond_init(&(cbp.notempty), NULL);

    pthread_create(&t_e1, NULL, enqueue30, &cbp);
    pthread_create(&t_e2, NULL, enqueue30, &cbp);
    pthread_create(&t_d1, NULL, dequeue10, &cbp);
    pthread_create(&t_d2, NULL, dequeue10, &cbp);
    pthread_create(&t_d3, NULL, dequeue10, &cbp);
    pthread_create(&t_d4, NULL, dequeue10, &cbp);
    pthread_create(&t_d5, NULL, dequeue10, &cbp);
    pthread_create(&t_d6, NULL, dequeue10, &cbp);

    pthread_join(t_e1, NULL);
    pthread_join(t_e2, NULL);
    pthread_join(t_d1, NULL);
    pthread_join(t_d2, NULL);
    pthread_join(t_d3, NULL);
    pthread_join(t_d4, NULL);
    pthread_join(t_d5, NULL);
    pthread_join(t_d6, NULL);

    pthread_mutex_destroy(&(cbp.buf_lock));
    pthread_cond_destroy(&(cbp.notfull));
    pthread_cond_destroy(&(cbp.notempty));

    return 0;
}
