#include <stdio.h>
#include "bankserv.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define PORT 55535
int main(int argc, char* argv[]) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    //CHECK THESE LINES FOR ERRORS
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1){
        printf("BIND FAILED");
        return -1;
    }
    if(-1==listen(sockfd, 12)){
        printf("LISTEN FAILED");
        return -1;
    }
    char buffer[1024];
    int conn = accept(sockfd, (struct sockaddr*)NULL , NULL);
    printf("conn %d\n",conn);
    while(1){
        read(conn,buffer,1024);
        write(conn,"HELLO WORLD\n", sizeof("HELLO WORLD"));
        printf("GOT CONNECTION: %s\n",buffer);
        printf("%d\n",strcmp(buffer, "hello"));
        if(strcmp(buffer, "hello")==0){
            close(conn);
            break;
        }

    }
    return 0;
}