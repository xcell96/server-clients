#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "constants.h"
#include "netutils.h"

void* handle_client(void* arg) {
    int client_socket = *((int*)arg);
    free(arg);
    char buffer[BUFSIZE];
    ssize_t n;

    printf("Connection opened.\n");

    while((n = read(client_socket, buffer, BUFSIZE - 1)) > 0){
        buffer[n] = '\0';
        printf("Received: %s", buffer);
        fflush(stdout);
    }

    close(client_socket);
    printf("Connection closed.\n");

    return NULL;
}

int main(){

    struct socketinfo* sock = init_socket();
    if(sock == NULL) { return 1; }

    int opt = 1;
    setsockopt(sock->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int res =
        bind(
            sock->sockfd,
            (struct sockaddr*)&sock->sock_addr,
            sizeof(sock->sock_addr)
        );

    if(res == -1) {
        close(sock->sockfd);
        free(sock);
        perror("binding socket failed");
        return 1;
    }

    if(listen(sock->sockfd, SOMAXCONN) == -1) {
        close(sock->sockfd);
        free(sock);
        perror("listen failed");
        return 1;
    }

    while(1){
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = sizeof(client_addr);

        int clientfd =
            accept(
                sock->sockfd,
                (struct sockaddr*)&client_addr,
                &client_len
            );
        if(clientfd == -1) { continue; }

        pthread_t t;
        int* pclient = malloc(sizeof(int));
        *pclient = clientfd;
        pthread_create(&t, NULL, handle_client, (void*)pclient);
    }

    close(sock->sockfd);
    free(sock);

    printf("Bye.\n");
}
