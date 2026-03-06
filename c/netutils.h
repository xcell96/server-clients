#pragma once
#include <arpa/inet.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct socketinfo {
    struct sockaddr_in sock_addr;
    socklen_t socket_len;
    int sockfd;
    char ip[INET_ADDRSTRLEN];
    uint16_t port;
};

struct client_context {
    struct socketinfo* client;
    char* server_ip;
    uint16_t server_port;
};

int
init_addr(struct sockaddr_in* addr, char* ip, uint16_t* port_listen);

struct socketinfo*
init_socket(char* ip, uint16_t* port_listen);

uint16_t
read_port();
