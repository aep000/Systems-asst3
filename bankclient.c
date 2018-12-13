#include <stdio.h> 
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <signal.h>
#include <netdb.h>
#include "bankclient.h"
int main(int argc, char * argv[]){
	if(argc != 3){
		printf("Bad Arguments expecting <Server Address> <Port>\n");
		return 0;
	}
	struct sockaddr_in serv_addr;
	int port = atoi(argv[2]);   
   	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   	if (sockfd < 0) {
      		perror("ERROR opening socket");
      		return 0;
   	}

   	struct hostent *server = gethostbyname(argv[1]);
	if(server==NULL){
		perror("Invalid Host Name\n");
		return 0;
	}
   	//serv_addr.sin_addr.s_addr = argv[1];
	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_port = htons(port);
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   	if (server == NULL) {
      		fprintf(stderr,"ERROR, no such host\n");
      		return 0;
   	}
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      		perror("ERROR connecting");
      		return 0;
   	}
	
	pthread_t tid;
	pthread_create(&tid, NULL, sockReadLoop,&sockfd);
	char buff[1024];
	char ch;
	int c = 0;
	while(read(0, &ch, 1) > 0){
		if(endFlag){
			printf("Connection Terminated\n");
			return 0;
		}
		if(ch == '\n'){
			buff[c]='\0';
			char * temp = strstr(buff," ");
			char * arg;
			if (temp == NULL){
				arg = NULL;	
			}
			else{
				arg = temp+1;
				*temp = '\0';
			}
			char code = runCommand(buff,arg,sockfd);
			c=0;
			memset(buff,0,strlen(buff));
			if(code != 'i')
				sleep(2);
		}
		else{
			buff[c]=ch;
			c+=1;
		}
		if(endFlag){
			printf("Connection Terminated\n");
			return 0;
		}
		
 	}
}
