#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const char HOST[] = "0.0.0.0";
const short PORT_LISTEN = 4040;
const size_t BUFSIZE = 128;

int main(int argc, char* argv[]){

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_LISTEN);
    if(inet_pton(AF_INET, HOST, &addr.sin_addr) == -1){
        perror("inet_pton failed");
        return 1;
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) { return 1; }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) { return 1; }
    if(listen(sockfd, SOMAXCONN) == -1) { return 1; }

    char buffer[BUFSIZE];
    ssize_t n;

    while(1){
        struct sockaddr_in client_addr = {0};
        socklen_t client_len = sizeof(client_addr);

        int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if(clientfd == -1) { continue; }

        while((n = read(clientfd, buffer, BUFSIZE - 1)) > 0){
            buffer[n] = '\0';
            printf("Received: %s", buffer);
        }

        close(clientfd);
    }

    close(sockfd);

}
