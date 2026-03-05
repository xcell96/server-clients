#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

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
    
    struct socketinfo* server = init_socket(&port);
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

    printf("Connected to %s:%d\n", ip, port);
    fflush(stdout);

    pthread_t t_recv, t_send;
    pthread_create(&t_recv, NULL, recv_thread, NULL);
    pthread_create(&t_send, NULL, send_thread, NULL);

    pthread_join(t_send, NULL);

    close(server->sockfd);
    free(server);

    printf("Bye.\n");
}

void* recv_thread(void* arg){
    char buffer[BUFSIZE];
    while(1){
        ssize_t n = read(sockfd, buffer, BUFSIZE-1);
        if(n <= 0){
            pthread_mutex_lock(&print_lock);
            printf("\nServer closed connection.\n");
            pthread_mutex_unlock(&print_lock);
            exit(0);
        }
        buffer[n] = '\0';
        pthread_mutex_lock(&print_lock);
        printf("\nServer: %s\nMessage: ", buffer);
        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
    }
    return NULL;
}

void* send_thread(void* arg){
    char msg[BUFSIZE];
    while(1){
        pthread_mutex_lock(&print_lock);
        printf("Message: ");
        fflush(stdout);
        pthread_mutex_unlock(&print_lock);

        if(!fgets(msg, BUFSIZE, stdin)) break;
        msg[strcspn(msg, "\n")] = '\0';

        if(strcmp(msg, "QUIT") == 0) break;

        send(sockfd, msg, strlen(msg), 0);
    }
    printf("Bye.\n");
    exit(0);
}
