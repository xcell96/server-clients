#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "netutils.h"
#include "constants.h"

int
init_addr(struct sockaddr_in* addr, char* ip, uint16_t* port_listen){
    if(addr == NULL || port_listen == NULL) return 1;

    const char* host = (ip == NULL) ? HOST : ip;

    addr->sin_family = AF_INET;
    addr->sin_port =
        port_listen == NULL ? htons(PORT_LISTEN) : htons(*port_listen);

    if(inet_pton(AF_INET, host, &addr->sin_addr) <= 0){
        perror("inet_pton failed");
        return 1;
    }

    return 0;
}

struct socketinfo*
init_socket(char* ip, uint16_t* port_listen) {
    struct socketinfo* s =
        (struct socketinfo*)malloc(sizeof(struct socketinfo));

    if(s == NULL) return NULL;

    if(init_addr(&s->sock_addr, ip, port_listen)) {
        free(s);
        return NULL;
    }

    s->socket_len = sizeof(s->sock_addr);

    s->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(s->sockfd == -1) {
        perror("socket creation failed");
        free(s);
        return NULL;
    }

    inet_ntop(AF_INET, &s->sock_addr.sin_addr, s->ip, sizeof(s->ip));

    s->port = *port_listen;

    return s;
}

uint16_t read_port() {
    uint16_t port;

    printf("Port: ");
    scanf("%hu", &port);
    getchar();

    return port;
}
