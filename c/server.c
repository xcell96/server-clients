#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "constants.h"
#include "netutils.h"

void* handle_client(void* arg);

int main(){

    // get listen port from user
    uint16_t port = read_port();

    // initialize the socket
    struct socketinfo* sock = init_socket(NULL, &port);
    if(sock == NULL) { return 1; }

    int opt = 1;
    setsockopt(sock->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // binding name to socket
    int res =
        bind(
            sock->sockfd,
            (struct sockaddr*)&sock->sock_addr,
            sizeof(sock->sock_addr)
        );

    if(res != 0) {
        close(sock->sockfd);
        free(sock);
        perror("binding socket failed");
        return 1;
    }

    // making socket listen for incoming connections
    if(listen(sock->sockfd, SOMAXCONN) == -1) {
        close(sock->sockfd);
        free(sock);
        perror("listen failed");
        return 1;
    }

    printf("Listening on %s:%d...\n", sock->ip, sock->port);

    // listen forever
    while(1){
        // preparing struct for client data
        struct client_context* ctx =
            (struct client_context*)malloc(sizeof(struct client_context));
        struct socketinfo* client =
            (struct socketinfo*)malloc(sizeof(struct socketinfo));
        client->socket_len = sizeof(client->sock_addr);

        // accepting the connection
        client->sockfd =
            accept(
                sock->sockfd,
                (struct sockaddr*)&client->sock_addr,
                &client->socket_len
            );
        if(client->sockfd == -1) {
            free(client);
            continue;
        }

        // further populating the struct with client data
        getpeername(
            client->sockfd,
            (struct sockaddr*)&client->sock_addr,
            &client->socket_len
        );
        inet_ntop(
            AF_INET,
            &client->sock_addr.sin_addr,
            client->ip,
            sizeof(client->ip)
        );
        client->port = ntohs(client->sock_addr.sin_port);

        ctx->client = client;
        ctx->server_ip = sock->ip;
        ctx->server_port = port;

        // making a separate thread for handling the connection
        pthread_t t;
        pthread_create(&t, NULL, handle_client, (void*)ctx);
    }

    // once waiting is done, close the local socket and free the memory
    close(sock->sockfd);
    free(sock);

    printf("Bye.\n");
}

void* handle_client(void* arg) {
    struct client_context* ctx = ((struct client_context*)arg);
    struct socketinfo* client = ctx->client;
    char* server_ip = ctx->server_ip;
    uint16_t server_port = ctx->server_port;
    free(arg);

    char buffer[BUFSIZE];
    char msg[16];
    int flag;
    ssize_t n;

    printf("Connection opened from %s:%d\n", client->ip, client->port);

    while((n = read(client->sockfd, buffer, BUFSIZE - 1)) > 0){
        buffer[n] = '\0';
        flag = 0;

        printf("[%s:%d -> %s:%d] Received: %s\n",
                client->ip,
                client->port,
                server_ip,
                server_port,
                buffer
        );

        if(strcmp(buffer, "sheep") == 0) {
            strcpy(msg, "baa");
            flag = 1;
        }else if(strcmp(buffer, "dog") == 0) {
            strcpy(msg, "woof");
            flag = 1;
        }else if(strcmp(buffer, "cow") == 0) {
            strcpy(msg, "moo");
            flag = 1;
        }else if(strcmp(buffer, "pig") == 0) {
            strcpy(msg, "oink");
            flag = 1;
        }

        if(flag) {
            send(client->sockfd, msg, strlen(msg)+1, 0);
            printf("[%s:%d -> %s:%d] Sent: %s\n",
                    server_ip,
                    server_port,
                    client->ip,
                    client->port,
                    msg
            );
        }
        
        fflush(stdout);
    }

    close(client->sockfd);
    printf("Connection to %s:%d closed.\n", client->ip, client->port);

    free(client);
    return NULL;
}
