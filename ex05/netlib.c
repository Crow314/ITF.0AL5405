#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "netlib.h"

int make_server_sock(int port)
{
    int server_sock;
    struct sockaddr_in addr;

    check((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)), "socket", -1);
    if (port != 0) {
      int value = 1;
      
      check((setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))), "setsockopt", -1);
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      addr.sin_addr.s_addr = INADDR_ANY;
      check(bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)), "bind", -1);
    }
    check(listen(server_sock, 0), "listen", -1);
    return server_sock;
}

int connect_dest(char *host_name, char *port)
{
    struct hostent *host;
    struct servent *service;
    struct sockaddr_in addr;
    int sock;

    addr.sin_family = AF_INET;
    if ((addr.sin_addr.s_addr = inet_addr(host_name)) == (in_addr_t) -1) {
      if ((host = gethostbyname(host_name)) != NULL) {
	memcpy((char *) &addr.sin_addr, host->h_addr, host->h_length);
      } else {
	fprintf(stderr, "unknown host %s\n", host_name);
	return -1;
      }
    }
    if ((addr.sin_port = htons(atoi(port))) == 0) {
      if ((service = getservbyname(port, "tcp")) != NULL) {
	addr.sin_port = service->s_port;
      } else {
	fprintf(stderr, "unrecognized port %s\n", port);
	return -1;
      }
    }
    check((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)), "socket", -1);
    show_addr("Connecting to", &addr);
    check(connect(sock, (struct sockaddr *)&addr, sizeof(addr)), "connect", -1);

    return sock;
}

#ifdef DEBUG
void show_addr(const char *msg, struct sockaddr_in *addr)
{
  fprintf(stderr, "%s %s:%d\n", msg, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}
#endif
