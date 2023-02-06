#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>      
#include <cstdlib>
#include <signal.h>
#include <ctime>
#include <cmath>
#include "../filewatcher.cpp"
#include "../dir_manager.cpp"
#include "../common/common.hpp"


using namespace std;

void verificaRecebimentoParametros(int argc);

int upload_file_client(int sock, char username[],std::string file_path, bool sync);

int download_file_client(int sock,char username[], std::string file_path);

void* handle_updates(void *arg);

void handle_ctrlc(int s);