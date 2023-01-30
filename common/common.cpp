#include "./common.hpp"
#include <mutex>

mutex pkt_mtx;

using namespace std;

void readSocket(PACKET *pkt, int sock){
    int n = 0;
	bool erro = false;

    while(n != sizeof(*pkt) && erro != true)
    {
    	/* read from the socket */
		n = read(sock, pkt, sizeof(*pkt)); 

		if (n < 0) 
		{
			cout<<"ERROR reading from socket"<<endl;
			erro = true;
		}	
    }
}

void sendMessage(string message, int seqn, int messageType,int fragmentos, char username[], int sockfd)
{
	PACKET pkt;
	
	pkt_mtx.lock();
	pkt.type = messageType;
	pkt.seqn = seqn;
	pkt.total_size = fragmentos;
	strcpy(pkt.user, username);
	strcpy(pkt._payload, message.c_str());
	pkt.length = strlen(pkt._payload);
	pkt_mtx.unlock();

	int n = write(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("ERROR writing to socket\n");
}

