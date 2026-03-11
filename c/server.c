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

#define MAX_CLIENTS 8

int clients[MAX_CLIENTS];
size_t client_counter = 1;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg);
void broadcast(const char* msg, size_t len);

int main(){

    memset(clients, -1, sizeof(int) * MAX_CLIENTS);

    char ip[INET_ADDRSTRLEN];
    printf("Bind IP (check network topology first): ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = '\0';

    // get listen port from user
    uint16_t port = read_port();

    // initialize the socket
    struct socketinfo* sock = init_socket(ip, &port);
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
        ctx->client_no = client_counter++;

        pthread_mutex_lock(&clients_mutex);

        if(client_counter < MAX_CLIENTS) {
            clients[client_counter] = client->sockfd;
        }

        pthread_mutex_unlock(&clients_mutex);

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
    size_t client_no = ctx->client_no;
    free(arg);

    char buffer[BUFSIZE];
    char msg[BUFSIZE];
    ssize_t n;

    printf("Connection %lu opened from %s:%d\n", client_no, client->ip, client->port);

    while((n = read(client->sockfd, buffer, BUFSIZE - 1)) > 0){
        buffer[n] = '\0';

        printf("[%s:%d -> %s:%d] Received: %s\n",
                client->ip,
                client->port,
                server_ip,
                server_port,
                buffer
        );

        if(strcmp(buffer, "sheep") == 0) {
            strcpy(msg, "baa");
        }else if(strcmp(buffer, "dog") == 0) {
            strcpy(msg, "woof");
        }else if(strcmp(buffer, "cow") == 0) {
            strcpy(msg, "moo");
        }else if(strcmp(buffer, "pig") == 0) {
            strcpy(msg, "oink");
        }else{
            strcpy(msg, buffer);
        }

        snprintf(buffer, sizeof(buffer), "[%s:%d -> %s:%d] Client %lu: %s\n",
                server_ip,
                server_port,
                client->ip,
                client->port,
                client_no,
                msg);

        broadcast(buffer, strlen(buffer)+1);
        printf("%s", buffer);
        
        fflush(stdout);
    }

    pthread_mutex_lock(&clients_mutex);

    for(size_t i = 0; i < client_counter; i++){
        if(clients[i] == client->sockfd){
            clients[i] = -1;
            client_counter--;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    close(client->sockfd);
    printf("Connection to %s:%d closed.\n", client->ip, client->port);
    snprintf(buffer, sizeof(buffer), "Client %lu left.\n", client_no);
    broadcast(buffer, strlen(buffer)+1);

    free(client);
    return NULL;
}

void broadcast(const char* msg, size_t len) {
    pthread_mutex_lock(&clients_mutex);

    for(size_t i = 0; i <= client_counter; i++) {
        if(clients[i] != -1)
            send(clients[i], msg, len, 0);
    }

    pthread_mutex_unlock(&clients_mutex);
}
