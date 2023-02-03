#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include "../login/login.hpp"
#include "../dir_manager.cpp"

void verificaRecebimentoIP(int argc,char *argv[]);

void imprimeServerError(void);

void *ThreadClient(void *arg);