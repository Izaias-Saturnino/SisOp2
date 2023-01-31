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
#include "../login/login.hpp"

void verificaRecebimentoIP(int argc,char *argv[]);

void imprimeServerError(void);

void *ThreadClient(void *arg);