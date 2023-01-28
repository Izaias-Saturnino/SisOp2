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
#include <cstdlib>
#include <signal.h>
#include <ctime>
#include "../packet.hpp"

using namespace std;

void writeSocket(int sockfd, char user[]);

void readSocket(char buffer[], int sockfd);

void sendMessage(string message, int seqn, int messageType,int fragmentos, char username[], PACKET pkt, int sockfd);