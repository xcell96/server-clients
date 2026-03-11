#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "constants.h"
#include "netutils.h"

int sockfd;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void* recv_thread(void* arg);
void* send_thread(void* arg);

int main(){

    char ip[INET_ADDRSTRLEN];
    printf("Server IP: ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = '\0';

    uint16_t port = read_port();
    
    struct socketinfo* server = init_socket(ip, &port);
    if(server == NULL) { return 1; }

    int res =
        connect(
            server->sockfd,
            (struct sockaddr*)&server->sock_addr,
            sizeof(server->sock_addr)
        );

    if(res == -1) {
        perror("connect failed");
        close(server->sockfd);
        free(server);
        return 1;
    }

    sockfd = server->sockfd;

    printf("Connected to %s:%d", ip, port);
    fflush(stdout);

    struct client_context* ctx =
        (struct client_context*)malloc(sizeof(struct client_context));
    if(ctx == NULL) {
        perror("malloc failed");
        close(server->sockfd);
        free(server);
        return 1;
    }
    ctx->server_ip = (char*)malloc(INET_ADDRSTRLEN);

    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);

    if(getsockname(sockfd, (struct sockaddr*)&local_addr, &addr_len)) {
        perror("getsockname failed");
        close(server->sockfd);
        free(server);
        return 1;
    } else {
        ctx->client = server;                           // who i'm talking to
        inet_ntop(AF_INET,
                  &local_addr.sin_addr, 
                  ctx->server_ip, 
                  sizeof(ctx->server_ip));              // my ip
        ctx->server_port = ntohs(local_addr.sin_port);  // my port
        printf("%s\n", ctx->server_ip);
    }


    pthread_t t_recv, t_send;
    pthread_create(&t_send, NULL, send_thread, (void*)ctx);
    pthread_create(&t_recv, NULL, recv_thread, (void*)ctx);

    pthread_join(t_send, NULL);
    free(ctx->server_ip);
    free(ctx);

    close(server->sockfd);
    free(server);

    printf("Bye.\n");
}

void* recv_thread(void* arg){
    struct client_context* ctx = (struct client_context*)arg;
    struct socketinfo* server = ctx->client;

    char buffer[BUFSIZE];
    while(1){
        ssize_t n = read(sockfd, buffer, BUFSIZE-1);
        if(n <= 0){
            printf("\n%s:%d closed connection.\n",
                    ctx->server_ip,
                    ctx->server_port);
            exit(0);
        }
        buffer[n] = '\0';

        pthread_mutex_lock(&print_lock);
        rl_save_prompt();
        rl_replace_line("", 0);
        rl_redisplay();

        /*printf("[%s:%d -> %s:%d] Server: %s\n",
                server->ip,
                server->port,
                ctx->server_ip,
                ctx->server_port,
                buffer);*/
        printf("%s", buffer);

        rl_restore_prompt();
        rl_redisplay();
        pthread_mutex_unlock(&print_lock);
    }
    return NULL;
}

void* send_thread(void* arg){
    struct client_context* ctx = (struct client_context*)arg;
    struct socketinfo* server = ctx->client;

    char prompt[100];
    snprintf(prompt, sizeof(prompt), "[%s:%d -> %s:%d] Message: ",
            ctx->server_ip,
            ctx->server_port,
            server->ip,
            server->port);

    while(1){
        char* msg = readline(prompt);
        if(msg == NULL) break;

        if(strcmp(msg, "QUIT") == 0) {
            free(msg);
            break;
        }

        send(sockfd, msg, strlen(msg)+1, 0);
        free(msg);
    }
    printf("Bye.\n");
    exit(0);
}
