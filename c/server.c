#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "constants.h"
#include "netutils.h"

int main(){

    struct socketinfo sock = init_socket();
    if(sock.sockfd == -1) { return 1; }

    int opt = 1;
    setsockopt(sock.sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int res =
        bind(
            sock.sockfd,
            (struct sockaddr*)&sock.sock_addr,
            sizeof(sock.sock_addr)
        );

    if(res == -1) {
        close(sock.sockfd);
        perror("binding socket failed");
        return 1;
    }

    if(listen(sock.sockfd, SOMAXCONN) == -1) {
        close(sock.sockfd);
        perror("listen failed");
        return 1;
    }

    char buffer[BUFSIZE];
    ssize_t n;

    while(1){
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = sizeof(client_addr);

        int clientfd =
            accept(
                sock.sockfd,
                (struct sockaddr*)&client_addr,
                &client_len
            );
        if(clientfd == -1) { continue; }

        printf("Connection opened.\n");

        while((n = read(clientfd, buffer, BUFSIZE - 1)) > 0){
            buffer[n] = '\0';
            printf("Received: %s", buffer);
            fflush(stdout);
        }

        close(clientfd);
        printf("Connection closed.\n");
    }

    close(sock.sockfd);
    printf("Bye.\n");
}
