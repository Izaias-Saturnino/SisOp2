#include "./common.hpp"

mutex pkt_mtx;

using namespace std;

int readpktnum = 0;
int sendpktnum = 0;

int readSocket(PACKET *pkt, int sock){
    int n = 0;

    while(n < sizeof(PACKET))
    {
    	/* read from the socket */
		int result = recv(sock, pkt+n, sizeof(PACKET)-n, 0);

		if (result >= 0)
		{
			n += result;
		}
    }
	if(pkt->type == 0 || pkt->type > 60 || n != sizeof(*pkt) || n != sizeof(PACKET)){
		cout << "n: " << n << endl;
		cout << "pkt.type: " << pkt->type << endl;
		cout << "pkt._payload: " << pkt->_payload << endl;
		sleep(60);
	}

	cout << "pkt.type: " << pkt->type;
	cout << ". readpktnum: " << readpktnum;
	cout << ". sock: " << sock << endl;
	readpktnum++;

	return n;
}

void sendMessage(char message[256], int seqn, int messageType, int fragmentos, char username[], int sockfd)
{
	PACKET pkt;
	
	pkt_mtx.lock();
	pkt.type = messageType;
	pkt.seqn = seqn;
	pkt.total_size = fragmentos;
	strcpy(pkt.user, username);
	memcpy(pkt._payload, message,256);
	pkt.length = strlen(pkt._payload);

	int n = 0;

    while(n < sizeof(PACKET))
    {
    	/* read from the socket */
		int result = send(sockfd, (&pkt)+n, sizeof(PACKET)-n, 0);

		if (result >= 0)
		{
			n += result;
		}
    }

	if (n < 0)
		printf("ERROR writing to socket\n");
	if (n != sizeof(pkt) || n != sizeof(PACKET)){
		printf("ERROR writing all data to socket\n");
		cout << "n: " << n << endl;
		cout << "pkt.type: " << pkt.type << endl;
		cout << "pkt._payload: " << pkt._payload << endl;
		if(pkt.type == 0 || pkt.type > 60){
			sleep(60);
		}
	}

	cout << "pkt.type: " << pkt.type;
	cout << ". sendpktnum: " << sendpktnum;
	cout << ". sock: " << sockfd << endl;
	sendpktnum++;
	pkt_mtx.unlock();
}

