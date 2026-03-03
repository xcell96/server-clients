#include <stdio.h>
#include <stdlib.h>

#include "netutils.h"
#include "constants.h"

int
init_addr(struct sockaddr_in* addr){
    addr->sin_family = AF_INET;
    addr->sin_port = htons(PORT_LISTEN);

    if(inet_pton(AF_INET, HOST, &addr->sin_addr) <= 0){
        perror("inet_pton failed");
        return 1;
    }

    return 0;
}

struct socketinfo*
init_socket() {
    struct socketinfo* s =
        (struct socketinfo*)malloc(sizeof(struct socketinfo));

    if(init_addr(&s->sock_addr)) {
        free(s);
        return NULL;
    }

    s->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(s->sockfd == -1) {
        perror("socket creation failed");
        free(s);
        return NULL;
    }

    return s;
}
