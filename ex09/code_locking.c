#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct entry {
    struct entry *next;
    void *data;
};

struct list {
    struct entry *head;
    struct entry **tail;
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
    *list->tail = e;
    list->tail = &e->next;
    return (0);
}

struct entry *list_dequeue(struct list *list) {
    struct entry *e;

    if (list->head == NULL)
        return(NULL);
    e = list->head;
    list->head = e->next;
    if (list->head == NULL)
        list->tail = &list->head;
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

int main() {
    struct list *list;
    struct entry *entry;

    list = list_init();

    /* enqueue data */
    list_enqueue(list, strdup("first"));
    list_enqueue(list, strdup("second"));
    list_enqueue(list, strdup("third"));

    /* entry list */
    list_traverse(list, print_entry, NULL);

    /* delete "second" entry */
    entry = list_traverse(list, delete_entry, "second");
    if (entry != NULL) {
        free(entry->data);
        free(entry);
}

    /* dequeue data */
    while ((entry = list_dequeue(list)) != NULL) {
        printf("%s\n", (char *)entry->data);
        free(entry->data);
        free(entry);
    }
    free(list);
    return (0);
}
