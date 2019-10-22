//<Server.H>
#ifndef SERVER_H
#define SERVER_H
#include <stddef.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#define SHM_SIZE 1024
#define BUFFSIZE 130



typedef struct Account
{
  char Name[100];
  float Balance;
  int InSessionFlag;
}account;




int AccountSearch(account list[20], char name[100]);//Search for a specific account by Name;


int FindSpace(account list[20]);//return free index, or -1 if no free space


int StartSession(account list[20], char name[100], pthread_mutex_t *mptrCHILD);//Starts an account Session by changing the inSessionFlag


int FinishSession(account list[20], pthread_mutex_t *mptrCHILD);//End an account Session by changing the inSessionFlag to 0



int OpenAccount(account list[20], char name[100], int sockfd);//open a new account, return 1 if succ



int DepositAccount(account list[20], float amount, int sockfd);//add money to the account.returns 1 if successful 



int WithdrawAccount(account list[20], float amount, int sockfd);//Withdraw from the account (subtract money ) returns 1 if successful


void FunctionPicker(char* input, account list[20], int sockfd, pthread_mutex_t *mptrCHILD);// deals with commands, prases through out the bank functions


void * printAccounts(void* n);// print the banck every 2 seconds in its own thread



void ProcessFunction(int sockfd, account data[20], pthread_mutex_t *mptrCHILD);//every client gets its own process that runs these



int main(); // Main part - Executes Bank




#endif //SERVER_H
