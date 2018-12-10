#include <stdio.h>
#include "bankserv.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define PORT 55535
int main(int argc, char* argv[]) {
    if(argc != 2){
	perror("Invalid Format Expecting 1 argument: server <PORT>\n");
	exit(0);
    }
    pthread_mutex_init(&accountLock,NULL);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    serv_addr.sin_port = htons(atoi(argv[1]));
    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    //CHECK THESE LINES FOR ERRORS
    printf("Starting up on port: %s\n",argv[1]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1){
        printf("BIND FAILED\n");
        return -1;
    }
    if(-1==listen(sockfd, 12)){
        printf("LISTEN FAILED\n");
        return -1;
    }
    char buffer[1024];
    printf("ENTERING LOOP\n");
    while(1){
        int conn = accept(sockfd, (struct sockaddr*)NULL , NULL);
        printf("CONNECTED TO\n");
        Session* s = malloc(sizeof(Session));
        pthread_t tid;
        s->socketID = conn;
        pthread_create(&tid,NULL,sessionRunner,s);
        
    }
    return 0;
}
