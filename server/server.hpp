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

void verificaRecebimentoIP(int argc,char *argv[]);

void imprimeServerError(void);

void writeSocket(char buffer[], int sockfd);

void readSocket(PACKET pkt, int sockfd);

void *readAndWriteSocket(void *arg);