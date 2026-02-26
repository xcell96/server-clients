#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

const char HOST[] = "127.0.0.1";
const short PORT = 4040;
const size_t BUFSIZE = 128;

int main(){
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, HOST, &addr.sin_addr) != 1){
        perror("inet_pton failed");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) return 1;

    int conn_result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if(conn_result == -1) {
        perror("connect failed");
        close(sockfd);
        return 1;
    }

    printf("Connected to server!\n");

    char msg[BUFSIZE];

    while(1){
        printf("Message: ");
        fgets(msg, BUFSIZE, stdin);
        if(!strcmp(msg, "QUIT\n")) break;
        send(sockfd, msg, strlen(msg), 0);
    }

    close(sockfd);
}
