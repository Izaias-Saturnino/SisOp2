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

	char *r = (char*)q;
    int i = 0;
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

	char *r = (char*)q;
    int i = 0;
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

	// if(pkt->type == 0 || pkt->type > 60 || n != sizeof(*pkt) || n != sizeof(PACKET)){
	// 	cout << "n: " << n << endl;
	// 	cout << "pkt.type: " << pkt->type << endl;
	// 	cout << "pkt._payload: " << pkt->_payload << endl;
	// 	sleep(60);
	// }

	cout << "pkt.type: " << pkt->type;
	cout << ". readpktnum: " << readpktnum;
	cout << ". pkt.seqn: " << pkt->seqn;
	cout << ". sock: " << sock << endl;
	readpktnum++;

	return n;
}

void sendMessage(char message[BUFFER_SIZE], int messageType, int sockfd)
{
	PACKET pkt;
	
	pkt_mtx.lock();
	pkt.type = messageType;
	pkt.seqn = sendpktnum;
	memcpy(pkt._payload, message, BUFFER_SIZE);

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
		// cout << "n: " << n << endl;
		// cout << "pkt.type: " << pkt.type << endl;
		// cout << "pkt._payload: " << pkt._payload << endl;
		// if(pkt.type == 0 || pkt.type > 60){
		// 	sleep(60);
		//}
	}

	cout << "pkt.type: " << pkt.type;
	cout << ". sendpktnum: " << sendpktnum;
	cout << ". sock: " << sockfd << endl;
	sendpktnum++;
	pkt_mtx.unlock();
}

vector<vector<char>> receiveFileData(int sock){
	PACKET pkt;
	vector<vector<char>> file_data = {};
	cout << "read7" << endl;
	readSocket(&pkt, sock);
	while (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO)
	{
		vector<char> buffer(BUFFER_SIZE);

		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			buffer[i] = pkt._payload[i];
		}
		file_data.push_back(buffer);

		
		cout << "read77" << endl;
		readSocket(&pkt, sock);
	}

	return file_data;
}

void writeFileData(vector<vector<char>> file_data, string directory, int remainder_file_size){
	ofstream file_received;
	file_received.open(directory, ios_base::binary);
	for (int i = 0; i < file_data.size(); i++)
	{
		for (int j = 0; j < file_data.at(i).size(); j++)
		{
			if (i == file_data.size() - 1)
			{
				if (j == remainder_file_size && remainder_file_size != 0)
				{
					cout << "remainder break" << endl;
					cout << "remainder_file_size: " << remainder_file_size << endl;
					break;
				}
			}
			char *frag = &(file_data.at(i).at(j));
			// printf("%x ", (unsigned char)file_data.at(i).at(j));
			file_received.write(frag, sizeof(char));
		}
	}
	file_received.close();
}

void receiveFile(int sock, string file_path, PACKET *pkt_addr){
	PACKET pkt = *(pkt_addr);

	string file_name = string(pkt._payload);

	cout << "read77" << endl;
	readSocket(&pkt, sock);
	uint32_t* file_size_addr = (uint32_t *) &(pkt._payload);
	uint32_t file_size = *file_size_addr;

	cout << "file_size: " << file_size << endl;

	vector<vector<char>> file_data = receiveFileData(sock);

	//write file data
	int remainder_file_size = file_size % BUFFER_SIZE;
	
	writeFileData(file_data, file_path, remainder_file_size);
}

void sendFile(int sock, string file_path){
	//open file
	ifstream file;
    cout << "sending file" << endl;
    cout << "file path: " << file_path << endl;
	file.open(file_path, ios_base::binary);
	if (!file.is_open())
	{
		cout << "error opening file" << "\n" << endl;
        cout << "write10" << endl;
        sendMessage(nullptr, MENSAGEM_FALHA_ENVIO, sock);
		return;
	}

	cout << "write11" << endl;
	sendMessage((char *)file_path.c_str(), MENSAGEM_ENVIO_NOME_ARQUIVO, sock);

	//get file size
	file.seekg(0, file.end);
	uint32_t file_size = file.tellg();
	cout << "file_size: " << file_size << "\n";
	file.clear();
	file.seekg(0);

	cout << "write122" << endl;
	sendMessage((char*)&file_size, MENSAGEM_ENVIO_TAMANHO_ARQUIVO, sock);
	cout << "file_size: " << file_size << endl;

	//send file pkts
	char buffer[BUFFER_SIZE];
	for (int i = 0; i < file_size; i += ((sizeof(buffer)))) // to read file
	{
		//cout << "i: " << i << endl;
		memset(buffer, 0, BUFFER_SIZE);
		file.read(buffer, sizeof(buffer));
		//for(int j = 0;j<BUFFER_SIZE; j++){
			//printf("%x ", (unsigned char)buffer[i]);
		//}
		
		cout << "write12" << endl;
		sendMessage(buffer, MENSAGEM_ENVIO_PARTE_ARQUIVO, sock);
	}
	cout << "write13" << endl;
	sendMessage(buffer, MENSAGEM_ARQUIVO_LIDO, sock);
	file.close();
	cout << "file received"
			<< "\n"
			<< endl;
}

string getFileName(string file_path){
	string file_name;

	int last_pos = file_path.find_last_of("/") + 1;
	if(last_pos == 0){
		file_name = file_path;
	}
	else{
		file_name = file_path.substr(last_pos);
	}
	return file_name;
}