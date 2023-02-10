#include "./common.hpp"

mutex pkt_mtx;

using namespace std;

int readpktnum = 0;
int sendpktnum = 0;

void serialize(PACKET *pkt, char data[sizeof(PACKET)])
{
    char *q = (char*)data;
    *q = pkt->type;
	q += sizeof(pkt->type);
	*q = pkt->seqn;
	q += sizeof(pkt->seqn);
	*q = pkt->total_size;
	q += sizeof(pkt->total_size);
	*q = pkt->file_byte_size;
	q += sizeof(pkt->file_byte_size);

    char *p = (char*)q;
    int i = 0;
    while (i < BUFFER_SIZE)
    {
        *p = pkt->user[i];
        p++;
        i++;
    }

	char *r = (char*)p;
    i = 0;
    while (i < BUFFER_SIZE)
    {
        *r = pkt->_payload[i];
        r++;
        i++;
    }
}

void deserialize(PACKET *pkt, char data[sizeof(PACKET)])
{
    char *q = (char*)data;
    pkt->type = *q;
	q += sizeof(pkt->type);
	pkt->seqn = *q;
	q += sizeof(pkt->seqn);
	pkt->total_size = *q;
	q += sizeof(pkt->total_size);
	pkt->file_byte_size = *q;
	q += sizeof(pkt->file_byte_size);

    char *p = (char*)q;
    int i = 0;
    while (i < BUFFER_SIZE)
    {
        pkt->user[i] = *p;
    	p++;
        i++;
    }

	char *r = (char*)p;
    i = 0;
    while (i < BUFFER_SIZE)
    {
        pkt->_payload[i] = *r;
        r++;
        i++;
    }
}

int readSocket(PACKET *pkt, int sock){
    int n = 0;

	char data[sizeof(PACKET)];

	//cout << "reading" << endl;

    while(n < sizeof(PACKET))
    {
    	/* read from the socket */
		int result = read(sock, data+n, sizeof(PACKET)-n);

		if (result >= 0)
		{
			n += result;
		}
    }

	//cout << "deserializing" << endl;

	deserialize(pkt, data);

	if(pkt->type == 0 || pkt->type > 60 || n != sizeof(*pkt) || n != sizeof(PACKET)){
		cout << "n: " << n << endl;
		cout << "pkt.type: " << pkt->type << endl;
		cout << "pkt._payload: " << pkt->_payload << endl;
		sleep(60);
	}

	cout << "pkt.type: " << pkt->type;
	cout << ". readpktnum: " << readpktnum;
	cout << ". pkt.seqn: " << pkt->seqn;
	cout << ". sock: " << sock << endl;
	readpktnum++;

	return n;
}

void sendMessage(char message[BUFFER_SIZE], uint32_t file_byte_size, int messageType, int fragmentos, char username[BUFFER_SIZE], int sockfd)
{
	PACKET pkt;
	
	pkt_mtx.lock();
	pkt.type = messageType;
	pkt.seqn = sendpktnum;
	pkt.total_size = fragmentos;
	strcpy(pkt.user, username);
	memcpy(pkt._payload, message, BUFFER_SIZE);
	pkt.file_byte_size = file_byte_size;

	char data[sizeof(PACKET)];

	serialize(&pkt, data);

	int n = 0;

    while(n < sizeof(PACKET))
    {
    	/* read from the socket */
		int result = write(sockfd, data+n, sizeof(PACKET)-n);

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

