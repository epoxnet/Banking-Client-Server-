//<client.h>
#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#define BUFFSIZE 130


void error(const char *msg);//function used to report errors with an appropriate message.




void sig_hand(int signo);//sig handler




int checkConnection(int sd);//spammed periodically to check whether the server is still running, otherwise terminates the client.


void * readClient(void *fd);//thread function to read client inputs



void * readServer(void *fd);//thread function to read the messages from the server for user




int main(int argc, char** argv);// Main - Executing the <client.c>




#endif //CLIENT_H
