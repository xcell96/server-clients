#include <stdio.h>

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

struct socketinfo
init_socket() {
    struct socketinfo s = {0};
    s.sockfd = -1;

    if(init_addr(&s.sock_addr)) { return s; }
    s.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(s.sockfd == -1) { perror("socket creation failed"); }

    return s;
}
