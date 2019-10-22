//<Server.C>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include "server.h"





static account list[20]={{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}};
int currentAccount= -1; //Number of account being work on curently
int maxAccounts=20; //Max number of accounts
int totalAccounts=0; //total number of accounts being opened


pthread_mutex_t mutex[20];//locks for Bank
pthread_mutex_t *mptr;
pthread_mutexattr_t matr;

int mshared_mem_id; //Shared Memory 


int AccountSearch(account list[20], char name[100])
{
  int item;
    for(item=0; item<maxAccounts; item++)
      {	
	if (strcmp(list[item].Name, name)==0)
	  //if the item contains the given value, return the item
	  {
	    return item;
	  }
      }
  /*if we went through the entire list and didn't find the *value, then return -1 signifying that the value wasn't found*/
  return -1;
}
	  

int FindSpace(account list[20])
{
  int item;
  for(item=0; item<maxAccounts; item++)
    {
      if(strcmp(list[item].Name, "")==0)
	//if the item is free, return the item
	{
	  return item;
	}
    }
  //else
  return -1;
}


int StartSession(account list[20],char name[100],pthread_mutex_t *mptrCHILD)
{
  int item;
  for(item=0;item<maxAccounts;item++)
    {
      if(strcmp(list[item].Name, name)==0)
	//if the item contains the right account, toggle session
	{
	  printf("Starting the session for account %d !\n", item);
	  pthread_mutex_lock(&mptrCHILD[item]);
	  list[item].InSessionFlag=1;
	  currentAccount=item;
	  return 1;
	}
    }
  //if we went through entire list and didn't find the account, then return 0
  printf("Account does not exist, can't start.\n");
  return 0;
}
	  



int FinishSession(account list[20], pthread_mutex_t *mptrCHILD)
{
  printf("Finished Session!\n");
  list[currentAccount].InSessionFlag=0;
  pthread_mutex_unlock(&mptrCHILD[currentAccount]);
  currentAccount=-1;
  return 1;
}


int DepositAccount(account list[20], float amount, int sockfd)
{
  list[currentAccount].Balance= list[currentAccount].Balance + amount;
  write(sockfd, "Deposited\n.",strlen("Deposited\n."));
  return 1;
}


int WithdrawAccount(account list[20],float amount, int sockfd)
{
  if(amount > list[currentAccount].Balance)
    //if trying to withdraw more than you have ! 
    {
      char* message= "There is not enough balance to withdraw this amount. \n";
      write(sockfd, message, strlen(message));
      return 0;
    }
  //else
  list[currentAccount].Balance= list[currentAccount].Balance - amount;
  write(sockfd,"withdrawed\n.",strlen("withdrawed\n."));
  return 1;
}


int OpenAccount(account list[20], char name[100], int sockfd)
{
  int tempsearch = AccountSearch(list, name);
  if(tempsearch == -1)
    {
      int tempindex = FindSpace(list);
      if(tempindex = -1)
	{
 	  char* message=" Cannot insert ! The bank account limit is full!\n";
	  write(sockfd, message, strlen(message));
	  return 0;
	}
      strcpy(list[tempindex].Name, name);//open account
      totalAccounts++;//increment account total
      // char* message = "Account creation susseful\n";
      char* mess= (char*)malloc(1000*sizeof(char));
      sprintf(mess,"Account created successfully, account name: %s and balance: %8.2f \n", list[tempsearch].Name, list[tempsearch].Balance);
      write(sockfd, mess, strlen(mess));
      free(mess);
      return 1;
    }
  else
    {
      char* message="Account exists! Unsuccessful.\n";
      write(sockfd, message, strlen(message));
    }
  return 0;
}



void Functionpicker(char* input, account list[20], int sockfd, pthread_mutex_t *mptrCHILD)// Deals with Commands, prases through and goes to bank functions
{
  char *token;
  token = strtok ( input, " ");

  if(strcmp(token, "open")==0)
    {
      if(currentAccount==-1)
	{
	  token = strtok (NULL," "); //read the Name
	  char name[100];
	  strcpy(name, token);
	  OpenAccount(list, name, sockfd);
	}
      else
	{
	  char* message=("Cannot open new account until this session is finished\n");
	    write(sockfd, message, strlen(message));
	}
    }
  else if(strcmp( token, "start")==0)//start account
    {
      if(currentAccount != -1)
	{
	char* message="Cannot start new account until this session is finished\n";
	write(sockfd, message, strlen(message));
      }
      else
	{
	  token= strtok (NULL, " "); // read the Name
	  char name[100];
	  strcpy(name, token);
	  if (StartSession(list, name,mptrCHILD))
	    {
	char* message="Account Started!\n";
	write(sockfd, message, strlen(message));
	    }
	  else
	    {
	      char* message= "Error!couldn't start account!\n";
	      write(sockfd, message, strlen(message));
	    }
	}
    }
  else if(strcmp(token, "Deposit")==0)//Deposit
    {
      if(currentAccount!=-1)
	{
	  token = strtok( NULL," "); // get amount
	  float amount = atof(token);
	  DepositAccount(list, amount, sockfd);
	  char* message= "Deposited.\n";
	  write(sockfd, message, strlen(message));
	}
      else
	{
	  char* message=" Please start a session first.\n";
	  write(sockfd, message, strlen(message));
	}
    }
  else if(strcmp(token, "withdraw")==0)//Withdraw
    {
      if(currentAccount != -1)
	{
	  token = strtok ( NULL, " "); // get amount
	  float amount = atof(token);
	  WithdrawAccount(list, amount, sockfd);
	}
      else
	{
	  char * message= ( "Please start a session first.\n");
            write (sockfd, message, strlen(message));
	}
    }
  else if(strcmp(token, "Balance")==0)// Balance
    {
      char * message=(char*)malloc(1000*sizeof(char));
      if(currentAccount != -1)
	{
	  sprintf( message, "Account Balance= %8.2f \n", list[currentAccount].Balance);
	  write(sockfd, message, strlen(message));
	}
      else
	{
	  sprintf(message,"Not in session. Cannot display any balance\n");
	  write(sockfd, message, strlen(message));
	}
    free(message);
    }
  else if(strcmp(token, "finish")==0) //Finish
    {
      if(currentAccount==-1)
	{
	  char * message= "Please start a session first. \n";
	  write(sockfd, message, strlen(message));
	}
      else if(FinishSession(list,mptrCHILD))
	{
	  char* message= "Session finished.\n";
	  write(sockfd, message,strlen(message));
	}
      else
	{
	  char* message = " Error!\n";
	  write(sockfd, message, strlen(message));
	}
    }
  else
    {
      char* message=(" Error! Please Enter a Valid Command.\n"); // if user enter random inputs
      write(sockfd, message, strlen(message));
    }
}


void* printAccounts(void* n) // print the bank every 2 seconds in its own thread
{
  key_t key; // to access shared memory
  if((key= ftok("server.c",'8'))== -1)
    {
      exit(1);
    }
  account* list;
  int shmid;

  while(1)//loops infintly until hit the stop point!
    {

      sleep(2); // print every 2 seconds
      printf("\n");
      printf("\n");
      printf("===============================BANK ACCOUNTS==============================\n");
      printf("\n");
      //connecting to the segment

      if ((shmid = shmget(key, SHM_SIZE, 0777 | IPC_CREAT))== -1)
	{
	  exit(1);
	}
      // get a pointer to attach to segment

      list = (account*) shmat(shmid, (void*)0, 0);
      if (list ==(account*)(-1))
	  {
	    exit(1);
	  }
	  printf("SMS attached in main!\n");
	  int i;
	  for(i=0; i<maxAccounts;i++)
	    {
	      if(list[i].InSessionFlag==1)
		{
		  printf("Name: %s ", list[i].Name);
		  if(list[i].InSessionFlag==1)
		    printf("IN SESSION");
		  printf( " Balance: %8.2f\n", list[i].Balance); // printing all the info
		}
	    }
	  printf("\n");

	  printf("============================================================================\n");
	  printf("\n");

	  if(shmdt(list) == -1)
	    {
	      exit(1);
	    }
      return NULL;
    }
}


void ProcessFunction(int sockfd, account data[20], pthread_mutex_t *mptrCHILD)
{
    printf("Executing in child process!\n");
    char* buffer= (char*)malloc(BUFFSIZE*sizeof(char));
    while(1)
      {
	bzero(buffer, BUFFSIZE); //Clean Out the Buffer
	read(sockfd, buffer, BUFFSIZE);
	if ((strlen(buffer)>0)&&(buffer[strlen(buffer)-1]=='\n'))
	  {
	    buffer[strlen(buffer)-1]='\0';
	  }
	if(strcmp("exit", buffer)==0)
	  {
	    if(currentAccount != -1)
	      FinishSession(data, mptrCHILD);
	    break;
	  }
	Functionpicker(buffer, data, sockfd, mptrCHILD);//prase user commands
      }
    printf("Client wants to close connection\n");// client wants to leave
    if (shmdt(data) == -1)
      {
	exit(1);
      }
    
    if (shmdt(mptrCHILD)==-1)
      {
	exit(1);
      }
    close(sockfd);
    return;
}

  void error(const char *msg)//exits and outputs errors
  {
    perror(msg);
    exit(1);
  }


  //--------------------MAIN------------------------

  int main()
  {
    printf("Server is starting ... \n");

    size_t mshm_size;
    key_t mkey, key;
    int k, z, i, shmid;
    account* data;
    struct addrinfo request, *servinfo; //initialize getaddrinfo
    mkey= ftok(" server.c", '7');

    mshm_size= 20*sizeof(pthread_mutex_t); //initialize shared memory
    if((mshared_mem_id = shmget(mkey,mshm_size, 0777 | IPC_CREAT))==-1)
      {
	exit(1);
      }
    else if((mptr = (pthread_mutex_t *)shmat(mshared_mem_id, (void*) 0,0))==NULL)
      {
	exit(1);
      }

    for(k=0;k<20;++k)
      {
	mptr[k]=mutex[k];
      }

    if(pthread_mutexattr_init(&matr)!=0)
      {
	exit(1);
      }
    else if(pthread_mutexattr_setpshared(&matr, PTHREAD_PROCESS_SHARED)!=0)
      {
	exit(1);
      }

    
    for(z=0; z<20; z++)
      {
	if (pthread_mutex_init(&mptr[z],&matr)!=0)
	  {
	    exit(1);
	  }
      }

    if(shmdt(mptr)==-1)
      {
	exit(1);
      }
    if((key=ftok("server.c", '8'))==-1)// make the key
      {
	exit(1);
      }
    if((shmid = shmget(key,SHM_SIZE,0777 | IPC_CREAT))==-1)//onnect to the segment
      {
	exit(1);
      }

    data=(account*) shmat(shmid,(void*)0,0); //attach to the segment
    if(data==(account*)(-1))
      {
	exit(1);
      }

    for(i=0;i<20; ++i)
      {
	data[i]=list[i]; 
      }
    if(shmdt(data)==-1)
      {
	exit(1); //detach from the segment
      }

    pthread_t tid; // thread for printing out account details every 2 sec
    pthread_create (&tid,NULL, &printAccounts, 0);

    request.ai_flags = AI_PASSIVE;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    int on=1;


    request.ai_protocol= 0;
    request.ai_addrlen = 0;
    int sd;

    request.ai_addr=NULL;
    request.ai_canonname=NULL;
    request.ai_next=NULL;

    getaddrinfo(NULL, "11115", &request, &servinfo);//getaddrinfo to connect to socket
    sd=socket(servinfo->ai_family,servinfo->ai_socktype, servinfo->ai_protocol);
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR, &on,sizeof(int));
    bind(sd, servinfo->ai_addr, servinfo->ai_addrlen);
    listen(sd, 100);//wait for connections from clients
    int fd,pid;
    struct sockaddr_in senderAddr;
    unsigned int ic=sizeof(senderAddr);
    while((fd=accept(sd,(struct sockaddr *)&senderAddr, &ic))) //while loop that spawns new processes
      {
	pid = fork();
	if(pid==0)
	  {
	    close(sd);
	    data = (account*) shmat(shmid,(void*)0,0);//attach to segment
	    if (data==(account*)(-1))
	      {
		exit(1);
	      }
	    printf("Client connected!\n)");//can start transaction operations

	    pthread_mutex_t *mdata=(pthread_mutex_t*)shmat(mshared_mem_id, (void*)0,0);
	    if(mdata==(pthread_mutex_t*)(-1))
	      {
		exit(1);
	      }
	    ProcessFunction(fd, data, mdata); //do stuff for each process
	    exit(0);
	  }
	else close(fd);
      }
   
  }//End Main
  
	  
    
	
    
	    
    
      

  
      
    
  
  
      
 
	  
