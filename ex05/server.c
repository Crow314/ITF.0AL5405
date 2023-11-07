/* server.c -- sample server
   usage: server [port]
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "netlib.h"

static char *get_line(char line[], size_t size, FILE *input)
{
  int length;
  if (fgets(line, size, input) == NULL) {
    return NULL;
  }
  length = strlen(line);
  while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
    line[--length] = '\0';
  }
  return line;
}

int main(int argc, char *argv[])
{
  int server_sock;
  int port;
  char line[128];

  if (argc != 2) {
    fprintf(stderr, "usage: %s port\n", argv[0]);
    return 1;
  }
  port = atoi(argv[1]);

  if ((server_sock = make_server_sock(port)) < 0) {
    return 1;
  }
  while (1) {
    int connection;
    FILE *in, *out;
    struct sockaddr_in client_addr;
    unsigned int addr_len = sizeof(client_addr);

    printf("Waiting for connection...\n");
    if ((connection = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
      perror("accept");
      return 1;
    }
    printf("Accepted connection from %s...\n", inet_ntoa(client_addr.sin_addr));

    if ((in = fdopen(connection, "r")) == NULL) {
      perror("fdopen(read)");
      return 1;
    }
    if ((out = fdopen(connection, "w")) == NULL) {
      perror("fdopen(write)");
      return 1;
    }
    while (get_line(line, sizeof(line), in) != NULL) {
      printf(">> %s\n", line);
      if ((rand() ^ line[0]) & 4) {
	printf("Skip!\n");
      } else {
	fprintf(out, "You said '%s'\r\n", line); fflush(out);
	printf("<< You said '%s'\n", line);
      }
    }
    printf(";Connection lost.\n");
    fclose(in);
    fclose(out);
  }
}
