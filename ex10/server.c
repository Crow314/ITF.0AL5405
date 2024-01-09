#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#include "log.h"

#define DEFAULT_SERVER_PORT 10000
#ifdef SOMAXCONN
#define LISTEN_BACKLOG SOMAXCONN
#else
#define LISTEN_BACKLOG 5
#endif

// ####
// Beggining of Job queue
// ####

struct entry {
    struct entry *next;
    void *(*function)(void *);
    void *arg;
};

struct list {
    struct entry *head;
    struct entry **tail;
    pthread_cond_t notempty;
    pthread_mutex_t m_lock;
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

int list_enqueue(struct list *list, void *(*function)(void *), void *arg) {
    struct entry *e;

    e = malloc(sizeof *e);
    if (e == NULL)
        return (1);
    e->next = NULL;
    e->function = function;
    e->arg = arg;

    pthread_mutex_lock(&list->m_lock);

    *list->tail = e;
    list->tail = &e->next;

    pthread_cond_signal(&list->notempty);
    pthread_mutex_unlock(&list->m_lock);
    return (0);
}

struct entry *list_dequeue(struct list *list) {
    struct entry *e;

    pthread_mutex_lock(&list->m_lock);

    while (list->head == NULL)
        pthread_cond_wait(&list->notempty, &list->m_lock);
    e = list->head;
    list->head = e->next;
    if (list->head == NULL)
        list->tail = &list->head;

    pthread_mutex_unlock(&list->m_lock);

    return (e);
}

// ####
// End of Job queue
// ####

// ####
// Beggining of Worker
// ####

void cleanup(void *arg) {
    int sock = *(int *)arg;
    close(sock);
    free(arg);
}

void *worker(void *arg) {
    struct entry *job;
    struct list *queue = (struct list*)arg;

    while (1) {
        job = list_dequeue(queue);
        pthread_cleanup_push(cleanup, job->arg);
        job->function(job->arg);
        pthread_cleanup_pop(1);
    }
}

// ####
// End of Worker
// ####

sigset_t sigset;
char *program_name = "server";

int open_accepting_socket(int port) {
    struct sockaddr_in self_addr;
    socklen_t self_addr_size;
    int sock, sockopt;

    memset(&self_addr, 0, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = INADDR_ANY;
    self_addr.sin_port = htons(port);
    self_addr_size = sizeof(self_addr);
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        log_fatal("accepting socket: %d", errno);
    sockopt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            &sockopt, sizeof(sockopt)) == -1)
        log_warning("SO_REUSEADDR: %d", errno);
    if (bind(sock, (struct sockaddr *)&self_addr, self_addr_size) < 0)
        log_fatal("bind accepting socket: %d", errno);
    if (listen(sock, LISTEN_BACKLOG) < 0)
        log_fatal("listen: %d", errno);
    return (sock);
}


void *sig_handler(void *arg) {
    int sig, err;

    err = sigwait(&sigset, &sig);
    if (err || (sig != SIGINT && sig != SIGTERM))
        abort();

    log_info("bye");
    exit(0);
}

void *protocol_main(void *arg) {
    char c[1];
    int sock = *(int *)arg;

    while (read(sock, c, sizeof c) > 0)
        /* ignore protocol process */;
    log_info("disconnected");
}

void main_loop(int accepting_socket, struct list *list) {
    int sock;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    for (;;) {
        client_addr_size = sizeof(client_addr);
        sock = accept(accepting_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (sock < 0)
            log_error("accept error: %d", errno);
        else {
            int *arg = malloc(sizeof(int));
            if (arg == NULL) { // Handle malloc error
                log_error("malloc error: %d", errno);
                close(sock);
                continue;
            }

            *arg = sock;
            list_enqueue(list, protocol_main, arg);
        }
    }
}

void usage(void) {
    fprintf(stderr, "Usage: %s [option]\n", program_name);
    fprintf(stderr, "option:\n");
    fprintf(stderr, "\t-d\t\t\t\t... debug mode\n");
    fprintf(stderr, "\t-p <port>\n");
    exit(1);
}

int main(int argc, char **argv) {
    char *port_number = NULL;
    int ch, sock, server_port = DEFAULT_SERVER_PORT;
    int debug_mode = 0;

    log_set_priority_max_level(LOG_INFO);
    while ((ch = getopt(argc, argv, "dp:")) != -1) {
        switch (ch) {
        case 'd':
            debug_mode = 1;
            log_set_priority_max_level(LOG_DEBUG);

            break;
        case 'p':
            port_number = optarg;
            break;
        case '?':
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (port_number != NULL)
        server_port = strtol(port_number, NULL, 0);

    /* server_portでlistenし，socket descriptorをsockに代入 */
    sock = open_accepting_socket(server_port);

    if (!debug_mode) {
        log_syslog_open(program_name, LOG_PID, LOG_LOCAL0);
        daemon(0, 0);
    }

    pthread_t t, t_w1, t_w2, t_w3;
    struct list *list;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    pthread_create(&t, NULL, sig_handler, NULL);

    list = list_init();
    pthread_create(&t_w1, NULL, worker, list);
    pthread_create(&t_w2, NULL, worker, list);
    pthread_create(&t_w3, NULL, worker, list);

    main_loop(sock, list);

    /*NOTREACHED*/
    return (0);
}
