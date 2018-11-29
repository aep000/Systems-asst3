#include <stdio.h>
#include "bankserv.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define PORT 55535
int main(int argc, char* argv[]) {
    pthread_mutex_init(&accountLock,NULL);
    printf("MUTEXED");
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    //CHECK THESE LINES FOR ERRORS
    printf("BINDING");
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
    printf("ENTERING LOOP");
    while(1){
        int conn = accept(sockfd, (struct sockaddr*)NULL , NULL);
        printf("CONNECTED TO");
        Session* s = malloc(sizeof(Session));
        pthread_t tid;
        s->socketID = conn;
        pthread_create(tid,sessionRunner,s,NULL);
    }
    return 0;
}