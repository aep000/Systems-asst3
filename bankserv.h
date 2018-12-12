//
// Created by Alex Parson on 11/28/18.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#define BREAKCOMMAND ' '
#define CREATE '1'
#define SERVE '2'
#define DEPOSIT '3'
#define WITHDRAW '4'
#define QUERY '5'
#define END '6'
#define QUIT '7'
#define ERROR_ACCOUNT_EXISTS  -1
#define ERROR_NOT_IN_ACCOUNT  -2
#define ERROR_ACCOUNT_DNE -3
#define ERROR_ALREADY_SERVING_ACCOUNT -4
#define ERROR_OVERDRAFT -5
#define ERROR_IN_SERVICE_MODE -6
#define QUIT_CONNECTION 2



#define CREATE_SUCCESS "Successfully created account %s\n"
#define DEPOSIT_SUCCESS "Deposited %lf Into Account %s.\nCurrent Balance Is: %lf\n"
#define SERVE_SUCCESS "Now Serving account %s\n"
#define WITHDRAW_SUCCESS "Withdrew %lf From Account %s.\nCurrent Balance Is: %lf\n"
#define QUERY_SUCCESS "Current Balance for Account %s: %lf\n"
#define END_SUCCESS "Ended Service Session For Account %s\n"

#define ALARM_MESSAGE "%s	%lf	%s\n"

typedef struct account{
    double balance;
    char * name;
    bool connected;
    struct account *next;
}Account;

typedef struct session{
    struct account * currentAccount;
    char * clientIP;
    bool inAccount;
    int socketID;
}Session;

typedef struct job{
    int socketID;
    int job;
    struct session * session;
    char * data;

}Job;

void printError(int code, Session * session);
int createCommand(char * input, Session * session);
int serveCommand(char * input, Session * session);
int depositCommand(char * input, Session * session);
int withdrawCommand(char * input, Session * session);
int queryCommand(char * input, Session * session);
int endCommand(char * input, Session * session);
int quitCommand(char * input, Session * session);
void alarmHandler(int);


Account * Accounts;
sem_t accountLock;

int runCommand(char* input, Session * session){
    int c = 0;
    char command = input[0];
    if(command==CREATE){
        return createCommand(input, session);
    }
    if(command==SERVE){
        return serveCommand(input, session);
    }
    if(command==DEPOSIT){
        return depositCommand(input, session);
    }
    if(command==WITHDRAW){
        return withdrawCommand(input, session);
    }
    if(command==QUERY){
        return queryCommand(input, session);
    }
    if(command==END){
        return endCommand(input, session);
    }
    if(command==QUIT){
        return quitCommand(input, session);
    }
}
void printError(int code, Session * session){
    char * error = "";
    if(code == ERROR_ACCOUNT_EXISTS){
        error= "Cannot Create Account, Account With That Name Already Exists\n";
        //printf("%s",error);
        write(session->socketID,error,strlen(error)+1);
        return;
    }
    if(code == ERROR_ACCOUNT_DNE){
        error="Cannot Serve Account, No Account Exists With That Name\n";
        //printf("%s",error);
        write(session->socketID,error,strlen(error)+1);
        return;
    }
    if(code == ERROR_ALREADY_SERVING_ACCOUNT){
        error = "There is already a user connected to this account try again later\n";
        //printf("%s",error);
        write(session->socketID,error,strlen(error)+1);
        return;
    }
    if(code == ERROR_NOT_IN_ACCOUNT){
        error = "Unable to fulfill request, you are not currently connected to an account\n";
        //printf("%s",error);
        write(session->socketID,error,strlen(error)+1);
        return;
    }
    if(code == ERROR_OVERDRAFT){
        error="Your withdrawal is too large, this account does not have enough money\n";
        write(session->socketID,error,strlen(error)+1);
        return;
    }
    if(code == ERROR_IN_SERVICE_MODE){
	error="Operation not permited, exit service mode to use this command\n";
        write(session->socketID,error,strlen(error)+1);
        return;
    }
}

void * sessionRunner(void* connection){
    Session * session = (Session *) connection;
    char buffer[1024];
    int c = 0;
    while(read(session->socketID, &buffer[c],1)>0){
        if(buffer[c]=='\0'){
		int code = runCommand(buffer,session);
		if(code == QUIT_CONNECTION){
            		close(session->socketID);
            		free(session);
            		return NULL;
        	}
        	else{
            		printError(code,session);
        	}
		c=0;
		continue;
	}
	c++;
    }

}

char * getData(char * input){
    return input+1;
}

int createCommand(char * input, Session * session){
    char * buffer = getData(input);
    sem_wait(&accountLock);
    Account * cursor = Accounts;
    if(Accounts==NULL){
	Account *  newAccount = malloc(sizeof(Account));
	printf("<%s>: Creating new account %s\n",session->clientIP,buffer);
	newAccount->name = malloc(strlen(buffer)+1);
	strcpy(newAccount->name,buffer);
	Accounts = newAccount;
  sem_post(&accountLock);
	char *dest = malloc(strlen(CREATE_SUCCESS)+strlen(buffer));
    	sprintf(dest, CREATE_SUCCESS,buffer);
    	write(session->socketID,dest,strlen(dest));
    	return 1;

    }
    while(cursor->next!=NULL){
        if(strcmp(cursor->name,buffer)==0){
	    sem_post(&accountLock);
            return ERROR_ACCOUNT_EXISTS;
        }
        cursor=cursor->next;
    }
    if(strcmp(cursor->name,buffer)==0){
	sem_post(&accountLock);
        return ERROR_ACCOUNT_EXISTS;
    }
    Account *  newAccount = malloc(sizeof(Account));
    printf("<%s>: Creating new account %s\n",session->clientIP,buffer);
    newAccount->name = malloc(strlen(buffer)+1);
    strcpy(newAccount->name,buffer);
    cursor->next=newAccount;
    sem_post(&accountLock);
    char *dest = malloc(strlen(CREATE_SUCCESS)+strlen(buffer));
    sprintf(dest, CREATE_SUCCESS,buffer);
    write(session->socketID,dest,strlen(dest));
    return 1;
}

int serveCommand(char * input, Session * session){
    if(session->inAccount){
	return ERROR_IN_SERVICE_MODE;
    }
    char * request = getData(input);
    sem_wait(&accountLock);
    Account * cursor = Accounts;
    while(cursor!=NULL){
        if(strcmp(cursor->name,request)==0){
            break;
        }
	cursor = cursor->next;
    }
    if(cursor==NULL){
	sem_post(&accountLock);
        return ERROR_ACCOUNT_DNE;
    }
    if(!cursor->connected){
        cursor->connected=true;
        session->currentAccount = cursor;
        session->inAccount = true;
	printf("<%s>: Serving %s\n",session->clientIP,session->currentAccount->name);
        char *dest = malloc(strlen(SERVE_SUCCESS)+strlen(session->currentAccount->name));
        sprintf(dest, SERVE_SUCCESS,session->currentAccount->name);
        write(session->socketID,dest,strlen(dest));

    }
    else{
	sem_post(&accountLock);
        return ERROR_ALREADY_SERVING_ACCOUNT;
    }
    sem_post(&accountLock);
    return 1;
}

int depositCommand(char * input, Session * session){
    char * request = getData(input);
    if(session->inAccount){
        sem_wait(&accountLock);
        double amount;
        sscanf(request, "%lf", &amount);
	      printf("<%s>: Depositing $%s into %s\n", session->clientIP, request, session->currentAccount->name);
        session->currentAccount->balance+=amount;
        sem_post(&accountLock);
	      char *dest = malloc(600);
        sprintf(dest, DEPOSIT_SUCCESS,amount,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return ERROR_NOT_IN_ACCOUNT;
    }

}


int withdrawCommand(char * input, Session * session){
    char * request = getData(input);
    if(session->inAccount){
      sem_wait(&accountLock);
        double amount;
        sscanf(request, "%lf", &amount);
        if(amount>session->currentAccount->balance){
            sem_post(&accountLock);
            return ERROR_OVERDRAFT;
        }
	printf("<%s>: Withdrawing $%s from %s\n", session->clientIP, request, session->currentAccount->name);
        session->currentAccount->balance-=amount;
        sem_post(&accountLock);
       	char *dest = malloc(600);
	sprintf(dest, WITHDRAW_SUCCESS,amount,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return ERROR_NOT_IN_ACCOUNT;
    }

}

int queryCommand(char * input, Session * session){
    if(session->inAccount){
        char *dest = malloc(600);
        sprintf(dest, QUERY_SUCCESS,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return ERROR_NOT_IN_ACCOUNT;
    }
}

int endCommand(char * input, Session * session){
    if(session->inAccount){
	sem_wait(&accountLock);
	      printf("<%s>: Ending service of account %s\n", session->clientIP, session->currentAccount->name);
        char * temp =session->currentAccount->name; 
        session->currentAccount->connected=false;
        session->currentAccount=NULL;
        session->inAccount=false;
        sem_post(&accountLock);
        char *dest = malloc(600);
        sprintf(dest, END_SUCCESS,temp);
        write(session->socketID,dest,strlen(dest));
        sem_post(&accountLock);
        return 1;
    }
    else{
        return ERROR_NOT_IN_ACCOUNT;
    }
}

int quitCommand(char * input, Session * session){
    printf("<%s>: Disconnecting\n", session->clientIP);
    if(session->currentAccount !=NULL){
      sem_wait(&accountLock);
    	session->currentAccount->connected=false;
    	session->currentAccount=NULL;
    	session->inAccount=false;
      sem_post(&accountLock);
    }
    return QUIT_CONNECTION;
}


void alarmHandler(int sig){
	sem_wait(&accountLock);
	Account * cursor = Accounts;
	if(cursor==NULL){
		printf("Diagnostics:\nNo Accounts\n");
	}
	else{
		printf("Diagnostics:\n");
		while(cursor !=NULL){
			char * inservice = "IN SERVICE";
			if(!cursor->connected){
				inservice="";
			}
			printf(ALARM_MESSAGE,cursor->name,cursor->balance,inservice);
			cursor = cursor->next;
		}
	}
	signal(SIGALRM, alarmHandler);
	alarm(15);
	sem_post(&accountLock);
}
