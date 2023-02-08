#include "./common.hpp"

mutex pkt_mtx;

using namespace std;

int readpktnum = 0;
int sendpktnum = 0;

void readSocket(PACKET *pkt, int sock){
    int n = 0;
	bool erro = false;

    while(n != sizeof(*pkt))
    {
    	/* read from the socket */
		int result = read(sock, pkt+n, sizeof(*pkt)-n);

		if (result >= 0) 
		{
			n += result;
		}

		cout << "n: " << n << endl;
		cout << "pkt.type: " << pkt->type << endl;
		cout << "pkt._payload: " << pkt->_payload << endl;
    }
	cout << "n: " << n << endl;
	cout << "pkt.type: " << pkt->type << endl;
	cout << "pkt._payload: " << pkt->_payload << endl;
	if(pkt->type == 0 || pkt->type > 60){
		sleep(60);
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
	if (n != sizeof(pkt)){
		printf("ERROR writing all data to socket\n");
		cout << "n: " << n << endl;
		cout << "pkt.type: " << pkt.type << endl;
		cout << "pkt._payload: " << pkt._payload << endl;
		if(pkt.type == 0 || pkt.type > 60){
			sleep(60);
		}
	}

	cout << "sendpktnum: " << sendpktnum;
	cout << ". sock: " << sockfd << endl;
	sendpktnum++;
	pkt_mtx.unlock();
}

