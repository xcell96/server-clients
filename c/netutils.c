#include <stdio.h>

#include "netutils.h"
#include "constants.h"

ssize_t
init_addr(struct sockaddr_in *addr){
    addr->sin_family = AF_INET;
    addr->sin_port = htons(PORT_LISTEN);

    if(inet_pton(AF_INET, HOST, &addr->sin_addr) == -1){
        perror("inet_pton failed");
        return 1;
    }

    return 0;
}
