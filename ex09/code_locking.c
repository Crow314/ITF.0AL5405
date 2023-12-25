#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;

struct entry {
    struct entry *next;
    void *data;
};

struct list {
    struct entry *head;
    struct entry **tail;
    pthread_cond_t notempty;
};

struct list *list_init(void) {
    struct list *list;

    list = malloc(sizeof *list);
    if (list == NULL)
        return (NULL);
    list->head = NULL;
    list->tail = &list->head;
    return (list);
}

int list_enqueue(struct list *list, void *data) {
    struct entry *e;

    e = malloc(sizeof *e);
    if (e == NULL)
        return (1);
    e->next = NULL;
    e->data = data;

    pthread_mutex_lock(&m_lock);

    *list->tail = e;
    list->tail = &e->next;

    pthread_cond_signal(&list->notempty);
    pthread_mutex_unlock(&m_lock);
    return (0);
}

struct entry *list_dequeue(struct list *list) {
    struct entry *e;

    pthread_mutex_lock(&m_lock);

    while (list->head == NULL)
        pthread_cond_wait(&list->notempty, &m_lock);
    e = list->head;
    list->head = e->next;
    if (list->head == NULL)
        list->tail = &list->head;

    pthread_mutex_unlock(&m_lock);

    return (e);
}

struct entry *list_traverse(struct list *list, int (*func)(void *, void *), void *user) {
    struct entry **prev, *n, *next;

    if (list == NULL)
        return (NULL);

    prev = &list->head;
    for (n = list->head; n != NULL; n = next) {
        next = n->next;
        switch (func(n->data, user)) {
        case 0:
            /* continues */
            prev = &n->next;
            break;
        case 1:
            /* delete the entry */
            *prev = next;
            if (next == NULL)
                list->tail = prev;
            return (n);
        case -1:
        default:
            /* traversal stops */
            return (NULL);
        }
    }
    return (NULL);
}

int print_entry(void *e, void *u) {
    printf("%s\n", (char *)e);
    return (0);
}

int delete_entry(void *e, void *u) {
    char *c1 = e, *c2 = u;

    return (!strcmp(c1, c2));
}

// Threads

void *enqueue30(void *arg) {
    struct list *list = (struct list*) arg;
    char buf[10];

    for (int i=1; i<=30; i++) {
        sprintf(buf, "%d", i);
        list_enqueue(list, strdup(buf));
        printf("%ld: enqueue %d\n", pthread_self(), i);
    }
}

void *dequeue10(void *arg) {
    struct list *list = (struct list*) arg;
    struct entry *entry;

    for (int i=0; i<10; i++) {
        entry = list_dequeue(list);
        printf("%ld: dequeue %s\n", pthread_self(), (char *)entry->data);
    }
}

int main() {
    struct list *list;
    struct entry *entry;
    pthread_t t_e1, t_e2, t_d1, t_d2, t_d3, t_d4, t_d5, t_d6;

    list = list_init();

    pthread_create(&t_e1, NULL, enqueue30, list);
    pthread_create(&t_e2, NULL, enqueue30, list);
    pthread_create(&t_d1, NULL, dequeue10, list);
    pthread_create(&t_d2, NULL, dequeue10, list);
    pthread_create(&t_d3, NULL, dequeue10, list);
    pthread_create(&t_d4, NULL, dequeue10, list);
    pthread_create(&t_d5, NULL, dequeue10, list);
    pthread_create(&t_d6, NULL, dequeue10, list);

    pthread_join(t_e1, NULL);
    pthread_join(t_e2, NULL);
    pthread_join(t_d1, NULL);
    pthread_join(t_d2, NULL);
    pthread_join(t_d3, NULL);
    pthread_join(t_d4, NULL);
    pthread_join(t_d5, NULL);
    pthread_join(t_d6, NULL);

    free(list);
    return (0);
}
