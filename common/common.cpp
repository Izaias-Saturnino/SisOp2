#include "./common.hpp"

mutex pkt_mtx;

using namespace std;

bool tryReadSocket(PACKET *pkt, int sock){
    int n = 0;
	bool erro = false;

	bool something_found = true;

    while(n != sizeof(*pkt) && erro != true)
    {
		if(n == 0){
			something_found = false;
			break;
		}
    	/* read from the socket */
		n = read(sock, pkt, sizeof(*pkt)); 

		if (n < 0) 
		{
			cout<<"ERROR reading from socket"<<endl;
			erro = true;
		}	
    }

	return something_found;
}

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

void sendMessage(char message[256], int seqn, int messageType,int fragmentos, char username[], int sockfd)
{
	PACKET pkt;
	
	pkt_mtx.lock();
	pkt.type = messageType;
	pkt.seqn = seqn;
	pkt.total_size = fragmentos;
	strcpy(pkt.user, username);
	memcpy(pkt._payload, message,256);
	pkt.length = strlen(pkt._payload);
	pkt_mtx.unlock();

	int n = write(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("ERROR writing to socket\n");
}

