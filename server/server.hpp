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

void insert_in_server_list(SERVER_COPY server_copy);

int get_new_id(vector<SERVER_COPY> servers);

void *answer_server_up(void *arg);

void send_list_of_servers(int other_server_socket);
void receive_list_of_servers(int other_server_socket);
int connect_to_main_server();

bool has_bigger_id(SERVER_COPY main_server_copy);
void broadcast_new_server(SERVER_COPY server_copy, int msg_type);
void send_election();
int host_cmp(char* ip, char* other_ip);
void *between_server_sync(void *arg);
void* timer(void *arg);
void* connection_timer(void *arg);