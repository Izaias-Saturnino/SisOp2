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
#include <cmath>
#include "../login/login.hpp"
#include "../dir_manager.cpp"

void verificaRecebimentoIP(int argc,char *argv[]);

void imprimeServerError(void);

void send_file_to_client(int sock, std::string file_path);

void *ThreadClient(void *arg);

void close_client_connections();

void handle_ctrlc(int s);

void check_main_server_up(int server_socket);

int request_id(int server_socket);