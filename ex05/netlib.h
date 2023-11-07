#ifndef NETLIB_H
#define NETLIB_H

#include <arpa/inet.h>

int make_server_sock(int port);
int connect_dest(char *host_name, char *port);

#ifdef DEBUG
void show_addr(const char *msg, struct sockaddr_in *addr);
#else
#define show_addr(msg, addr)
#endif

#define check(e, msg, errorval)  do { if ((e) < 0) { perror(msg); return (errorval); } } while(0)

#endif
