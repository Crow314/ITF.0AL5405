
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <syslog.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

#include "log.h" 

#define DEFAULT_SERVER_PORT 10000 
#ifdef SOMAXCONN 
#define LISTEN_BACKLOG SOMAXCONN 
#else 
#define LISTEN_BACKLOG 5 
#endif 

char *program_name = "server"; 

int 
open_accepting_socket(int port) 
{ 
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


void
protocol_main(int sock)
{
    char c[1];

    while (read(sock, c, sizeof c) > 0)
        /* ignore protocol process */;
    close(sock);
    log_info("disconnected");
}

void
main_loop(int accepting_socket)
{
    int sock;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;

    for (;;) {
        client_addr_size = sizeof(client_addr);
        sock = accept(accepting_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (sock < 0)
            log_error("accept error: %d", errno);
        else
            protocol_main(sock);
    }
}

void 
usage(void) 
{ 
        fprintf(stderr, "Usage: %s [option]\n", program_name); 
        fprintf(stderr, "option:\n"); 
        fprintf(stderr, "\t-d\t\t\t\t... debug mode\n"); 
        fprintf(stderr, "\t-p <port>\n"); 
        exit(1); 
} 

int 
main(int argc, char **argv) 
{ 
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


        main_loop(sock); 

        /*NOTREACHED*/ 
        return (0); 
}
