#include "./common.hpp"

mutex pkt_mtx;

using namespace std;

int readpktnum = 0;
int sendpktnum = 0;

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
	if(readpktnum != (*pkt).seqn){
		cout << "(*pkt).seqn: " << (*pkt).seqn << ". readpktnum: " << readpktnum << ". ";
	}
	cout << "readpktnum: " << readpktnum;
	cout << ". sock: " << sock << endl;
	readpktnum++;
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

	int n = write(sockfd, &pkt, sizeof(pkt));
	if (n < 0)
		printf("ERROR writing to socket\n");

	cout << "sendpktnum: " << sendpktnum;
	cout << ". sock: " << sockfd << endl;
	sendpktnum++;
	pkt_mtx.unlock();
}

