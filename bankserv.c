#include <stdio.h>
#include "bankserv.h"
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#define PORT 55535
int main(int argc, char* argv[]) {
    if(argc != 2){
	perror("Invalid Format Expecting 1 argument: server <PORT>\n");
	exit(0);
    }
    sem_init(&accountLock,0,1);
    signal(SIGALRM, alarmHandler);
    alarm(15);
    signal(SIGINT,interuptHandler);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_addr.sin_port = htons(atoi(argv[1]));
    struct sockaddr_in clientAddr;
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
    int clientLen = sizeof(clientAddr);
    while(1){
        int conn = accept(sockfd, (struct sockaddr*) &clientAddr, &clientLen);
	char * strAddr = inet_ntoa(clientAddr.sin_addr);
        printf("Incoming Connection from: %s\n",strAddr);
        Session* s = malloc(sizeof(Session));
        pthread_t tid;
	s->clientIP = malloc(strlen(strAddr+1));
	strcpy(s->clientIP,strAddr);
        s->socketID = conn;
	s->next = Sessions;
	s->last = NULL;
	if(Sessions)
		Sessions->last = s;
	Sessions = s;
        pthread_create(&tid,NULL,sessionRunner,s);

    }
    return 0;
}
