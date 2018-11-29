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

#define BREAKCOMMAND ' '
#define CREATE '1'
#define SERVE '2'
#define DEPOSIT '3'
#define WITHDRAW '4'
#define QUERY '5'
#define END '6'
#define QUIT '7'
#define ERROR_ACCOUNT_EXISTS  -1
#define NOT_IN_ACCOUNT_ERROR  -2
#define ACCOUNT_DNE_ERROR -3
#define ALREADY_SERVING_ACCOUNT_ERROR -4
#define OVERDRAFT_ERROR -5
#define QUIT_CONNECTION 2



#define CREATE_SUCCESS "Successfully created account"
#define DEPOSIT_SUCCESS "Deposited %lf Into Account %s.\nCurrent Balance Is: %lf"
#define SERVE_SUCCESS "Now Serving account %s"
#define WITHDRAW_SUCCESS "Withdrew %lf From Account %s.\nCurrent Balance Is: %lf"
#define QUERY_SUCCESS "Current Balance for Account %s: %lf"
#define END_SUCCESS "Ended Service Session For Account %s"



typedef struct account{
    double balance;
    char * name;
    bool connected;
    struct account *next;
}Account;

typedef struct session{
    struct account * currentAccount;
    bool inAccount;
    int socketID;
}Session;

typedef struct job{
    int socketID;
    int job;
    struct session * session;
    char * data;

}Job;

void printError(int code);
int createCommand(char * input, Session * session);
int serveCommand(char * input, Session * session);
int depositCommand(char * input, Session * session);
int withdrawCommand(char * input, Session * session);
int queryCommand(char * input, Session * session);
int endCommand(char * input, Session * session);
int quitCommand(char * input, Session * session);

Account * Accounts;
pthread_mutex_t * accountLock = PTHREAD_MUTEX_INITIALIZER;

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
void printError(int code){
    
}
void * sessionRunner(void* connection){
    Session * session = (Session *) connection;
    char buffer[1024];
    while(1){
        read(session->socketID, buffer,1024);
        int code = runCommand(buffer,session);
        if(code == QUIT_CONNECTION){
            close(session->socketID);
            free(session);
            return NULL;
        }
        else{
            printError(code);
        }

    }

}

char * getData(char * input){
    return input+1;
}

int createCommand(char * input, Session * session){
    char * buffer = getData(input);
    pthread_mutex_lock(accountLock);
    Account * cursor = Accounts;
    while(cursor->next!=NULL){
        if(strcmp(cursor->name,buffer)==0){

            return ERROR_ACCOUNT_EXISTS;
        }
    }
    if(strcmp(cursor->name,buffer)==0){
        return ERROR_ACCOUNT_EXISTS;
    }
    Account *  newAccount = malloc(sizeof(Account));
    newAccount->name=buffer;
    cursor->next=newAccount;
    pthread_mutex_unlock(accountLock);
    write(session->socketID,CREATE_SUCCESS, sizeof(CREATE_SUCCESS));
    return 1;
}

int serveCommand(char * input, Session * session){
    char * request = getData(input);
    pthread_mutex_lock(accountLock);
    Account * cursor = Accounts;
    while(cursor!=NULL){
        if(strcmp(cursor->name,request)==0){
            break;
        }
    }
    if(cursor==NULL){
        return ACCOUNT_DNE_ERROR;
    }
    if(!cursor->connected){
        cursor->connected=true;
        session->currentAccount = cursor;
        session->inAccount = true;
        char *dest = NULL;
        sprintf(dest, SERVE_SUCCESS,session->currentAccount->name);
        write(session->socketID,dest,strlen(dest));

    }
    else{
        return ALREADY_SERVING_ACCOUNT_ERROR;
    }
    pthread_mutex_unlock(accountLock);
    return 1;
}

int depositCommand(char * input, Session * session){
    char * request = getData(input);
    if(session->inAccount){
        double amount;
        sscanf(getData(request), "%lf", &amount);
        session->currentAccount->balance+=amount;
        char * dest=NULL;
        sprintf(dest, DEPOSIT_SUCCESS,amount,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return NOT_IN_ACCOUNT_ERROR;
    }

}


int withdrawCommand(char * input, Session * session){
    char * request = getData(input);
    if(session->inAccount){
        double amount;
        sscanf(getData(request), "%lf", &amount);
        if(amount>session->currentAccount->balance){
            return OVERDRAFT_ERROR;
        }
        session->currentAccount->balance-=amount;
        char * dest=NULL;
        sprintf(dest, WITHDRAW_SUCCESS,amount,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return NOT_IN_ACCOUNT_ERROR;
    }

}

int queryCommand(char * input, Session * session){
    if(session->inAccount){
        char * dest=NULL;
        sprintf(dest, QUERY_SUCCESS,session->currentAccount->name,session->currentAccount->balance);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return NOT_IN_ACCOUNT_ERROR;
    }
}

int endCommand(char * input, Session * session){
    if(session->inAccount){
        char * temp =session->currentAccount->name;
        pthread_mutex_lock(accountLock);
        session->currentAccount->connected=false;
        session->currentAccount=NULL;
        session->inAccount=false;
        pthread_mutex_unlock(accountLock);
        char * dest=NULL;
        sprintf(dest, END_SUCCESS,temp);
        write(session->socketID,dest,strlen(dest));
        return 1;
    }
    else{
        return NOT_IN_ACCOUNT_ERROR;
    }
}

int quitCommand(char * input, Session * session){
    session->currentAccount->connected=false;
    session->currentAccount=NULL;
    session->inAccount=false;
    return QUIT_CONNECTION;
}

