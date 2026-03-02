#pragma once
#include <sys/types.h>
#include <arpa/inet.h>

struct socketinfo {
    int sockfd;
    struct sockaddr_in sock_addr;
};

int
init_addr(struct sockaddr_in* addr);

struct socketinfo
init_socket();
