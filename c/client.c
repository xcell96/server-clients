#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "netutils.h"

int main(){
    
    struct socketinfo sock = init_socket();
    if(sock.sockfd == -1) { return 1; }

    int res =
        connect(
            sock.sockfd,
            (struct sockaddr*)&sock.sock_addr,
            sizeof(sock.sock_addr)
        );

    if(res == -1) {
        perror("connect failed");
        close(sock.sockfd);
        return 1;
    }

    printf("Connected to server!\n");

    char msg[BUFSIZE];

    while(1){
        printf("Message: ");
        fgets(msg, BUFSIZE, stdin);
        if(!strcmp(msg, "QUIT\n")) break;
        send(sock.sockfd, msg, strlen(msg), 0);
    }

    close(sock.sockfd);
    printf("Bye.\n");
}
