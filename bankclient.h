#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#define BREAKCOMMAND ' '
#define CREATE '1'
#define SERVE '2'
#define DEPOSIT '3'
#define WITHDRAW '4'
#define QUERY '5'
#define END '6'
#define QUIT '7'

char runCommand(char * command, char * argument, int sockfd);
int isOnlyDouble(const char* str);
void * sockReadLoop(void * sockfd);
int endFlag = 0;
int isOnlyDouble(const char* str)
{
    if(strcmp("0",str)==0)
	return 1;
    char* endptr = 0;
    strtod(str, &endptr);

    if(*endptr != '\0' || endptr == str)
        return 0;
    return 1;
}

char runCommand(char * command, char * argument, int sockfd){
	char * fixedCommand = command;
	char * output = malloc(1024);
	char com;
	if(strcasecmp(fixedCommand,"create")==0){
		com=CREATE;
	}
	if(strcasecmp(fixedCommand,"serve")==0){
		com=SERVE;
	}
	if(strcasecmp(fixedCommand,"deposit")==0){
		if(!isOnlyDouble(argument)){
			fprintf(stderr,"Deposit argument is not valid, should be double\n");
			return 'd';
		}
		com=DEPOSIT;
	}
	if(strcasecmp(fixedCommand,"withdraw")==0){
		if(!isOnlyDouble(argument)){
			fprintf(stderr,"Withdraw argument is not valid, should be double\n");
			return 'd';
		}
		com=WITHDRAW;
	}
	if(strcasecmp(fixedCommand,"query")==0){
		com=QUERY;
	}
	if(strcasecmp(fixedCommand,"end")==0){
		com=END;
	}
	if(strcasecmp(fixedCommand,"quit")==0){
		com=QUIT;
	}
	if(com == 0){
		printf("Invalid command\n");
		return 'i';
	}
	output[0]=com;
	output[1]='\0';
	if(argument != NULL)
		strcat(output,argument);
	write(sockfd,output,strlen(output)+1);
	free(output);
	if(com == QUIT){
		//close(sockfd);
		//endFlag = 1;
	}
	return com;

}

void * sockReadLoop(void * sockfd){
	int sock = *(int *) sockfd;
	char ch;
	char last;
	while(read(sock, &ch, 1) > 0){
		if(last == '\n')
			printf("<server>: ");
		printf("%c",ch);
		last = ch;
 	}
	endFlag = 1;
	
	raise(SIGINT);
	return NULL;
}
