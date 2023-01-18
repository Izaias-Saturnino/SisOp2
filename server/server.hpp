#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h> 
#include <iostream>

#define PORT 4000

void verificaRecebimentoIP(int argc,char *argv[]);

void imprimeServerError(void);

void obtemMensagem(char buffer[]);

void writeSocket(char buffer[], int sockfd);

void readSocket(char buffer[], int sockfd);

void *Thread(void *arg);