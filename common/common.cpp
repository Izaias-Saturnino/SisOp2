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

	//cout << "reading pkt" << endl;

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

	//cout << "pkt.type: " << pkt->type << ". readpktnum: " << readpktnum << ". pkt.seqn: " << pkt->seqn << ". sock: " << sock << endl;
	readpktnum++;

	return n;
}

void peekSocket(PACKET *pkt, int sock){
	char buffer[sizeof(PACKET)];

	int n = 0;

	while(n < sizeof(PACKET)){
		n = recv(sock, buffer, sizeof(PACKET), MSG_PEEK);
	}

	deserialize(pkt, buffer);
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

	//cout << "pkt.type: " << pkt.type << ". sendpktnum: " << sendpktnum << ". sock: " << sockfd << endl;
	sendpktnum++;
	pkt_mtx.unlock();
}

int connect_to_server(string server_ip, int PORT){

	hostent *server_host = gethostbyname((char*) server_ip.c_str());

	return connect_to_server(*server_host, PORT);
}

int connect_to_server(hostent server_host){
	return connect_to_server(server_host, DEFAULT_PORT);
}

int connect_to_server(hostent server_host, int PORT){
	int sockfd = -1;
    struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int counter = 0;
	while (sockfd == -1 && counter < MAX_RETRIES)
	{
		usleep(WAIT_TIME_BETWEEN_RETRIES);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		counter++;
	}
	if(counter == MAX_RETRIES){
		cout << "ERROR opening socket" << endl;
		return sockfd;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	inet_aton(server_host.h_name, &serv_addr.sin_addr);
	bzero(&(serv_addr.sin_zero), 8);

	counter = 0;
	while (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && counter < MAX_RETRIES)
	{
		usleep(WAIT_TIME_BETWEEN_RETRIES);
		counter++;
	}
	if(counter == MAX_RETRIES){
		close(sockfd);
		sockfd = -1;
		cout << "ERROR connecting" << endl;
	}
	return sockfd;
}

//checks for PACKET in socket
bool has_received_message(int sock){
	char buffer[sizeof(PACKET)];
	int n = recv(sock, buffer, sizeof(PACKET), MSG_PEEK);
	return n >= sizeof(PACKET);
}

vector<vector<char>> receiveFileData(int sock){
	PACKET pkt;
	vector<vector<char>> file_data = {};
	cout << "reading file data" << endl;
	readSocket(&pkt, sock);
	while (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO)
	{
		vector<char> buffer(BUFFER_SIZE);

		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			buffer[i] = pkt._payload[i];
		}
		file_data.push_back(buffer);

		
		//cout << "reading MENSAGEM_ENVIO_PARTE_ARQUIVO" << endl;
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

	//cout << "reading file name" << endl;
	readSocket(&pkt, sock);
	uint32_t* file_size_addr = (uint32_t *) &(pkt._payload);
	uint32_t file_size = *file_size_addr;

	//cout << "file_size: " << file_size << endl;

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
        sendMessage(nullptr, MENSAGEM_FALHA_ENVIO, sock);
		return;
	}

	cout << "sending MENSAGEM_ENVIO_NOME_ARQUIVO" << endl;
	sendMessage((char*) file_path.c_str(), MENSAGEM_ENVIO_NOME_ARQUIVO, sock);

	//get file size
	file.seekg(0, file.end);
	uint32_t file_size = file.tellg();
	cout << "file_size: " << file_size << "\n";
	file.clear();
	file.seekg(0);

	cout << "sending MENSAGEM_ENVIO_TAMANHO_ARQUIVO" << endl;
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
		
		//cout << "sending MENSAGEM_ENVIO_PARTE_ARQUIVO" << endl;
		sendMessage(buffer, MENSAGEM_ENVIO_PARTE_ARQUIVO, sock);
	}
	cout << "sending ACK after file send" << endl;
	sendMessage(buffer, ACK, sock);
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

void create_thread(
	pthread_t *__restrict thr1,
	const pthread_attr_t *__restrict attr,
	void *(*handle_updates) (void *),
	void *__restrict sockfd_sync)
{
	int i = pthread_create(thr1, attr, handle_updates, sockfd_sync);
	while (i != 0)
	{
		i = pthread_create(thr1, attr, handle_updates, sockfd_sync);
		cout << "ERROR creating thread. retrying..." << endl;
		cout << "ERROR number: " << i << endl;
	}
}

vector<SERVER_COPY> insert_in_server_list(SERVER_COPY server_copy, vector<SERVER_COPY> servers){
    bool found_server = false;
    for(int i = 0; i < servers.size(); i++){
        bool ip_equals = str_equals((char*) (server_copy.ip).c_str(), server_copy.ip.size(), (char*) (servers[i].ip).c_str(), servers[i].ip.size());
        bool port_equals = server_copy.PORT == servers[i].PORT;
        if(ip_equals && port_equals){
            found_server = true;
            servers[i].id = server_copy.id;
            break;
        }
    }
    
    if(!found_server){
        servers.push_back(server_copy);
    }

	return servers;
}

vector<SERVER_COPY> receive_list_of_servers(int server_socket, vector<SERVER_COPY> servers){
    cout << "receive_list_of_servers" << endl;
    PACKET pkt;
    peekSocket(&pkt, server_socket);
    while(pkt.type != SERVER_ITEM){
        readSocket(&pkt, server_socket);
        peekSocket(&pkt, server_socket);
    }
    while(pkt.type == SERVER_ITEM){
        cout << "reading SERVER_ITEM" << endl;
        SERVER_COPY server_copy = receive_server_copy(server_socket);
        //update_this_server_info(server_copy);
        servers = insert_in_server_list(server_copy, servers);
        cout << "peeking SERVER_ITEM" << endl;
        peekSocket(&pkt, server_socket);
    }
    cout << "LIST_SERVER_ITEM end" << endl;
    readSocket(&pkt, server_socket);
	
	return servers;
}

//server function
int connect_to_main_server(vector<SERVER_COPY> servers, SERVER_COPY this_server){
    int socket = -1;

    sort(servers.begin(), servers.end(), compare_id);

    for (int i = servers.size() - 1; i >= 0; i--)
    {
        if(servers[i].id > this_server.id){
            socket = connect_to_server(servers[i].ip, servers[i].PORT);
            if(socket != -1){
                break;
            }
        }
        else{
            break;
        }
    }

    return socket;
}

bool compare_id(SERVER_COPY copy1, SERVER_COPY copy2){
    return copy1.id < copy2.id;
}

SERVER_COPY receive_server_copy(int socket){
    SERVER_COPY server_copy;

    PACKET pkt;

    readSocket(&pkt, socket);
    memcpy((char*) &(server_copy.id), pkt._payload, sizeof(server_copy.id));
    readSocket(&pkt, socket);
    memcpy((char*) &(server_copy.PORT), pkt._payload, sizeof(server_copy.PORT));
    int str_size;
    readSocket(&pkt, socket);
    memcpy((char*) &(str_size), pkt._payload, sizeof(str_size));
    readSocket(&pkt, socket);
    server_copy.ip = (string) (pkt._payload);
    cout << "received server_copy.id: " << server_copy.id << endl;

    return server_copy;
}

bool str_equals(char* str1, int str1_size, char* str2, int str2_size){
    if(str1_size != str2_size){
        return false;
    }
    bool equals = true;
    for(int i = 0; i < str1_size; i++){
        if(str1[i] != str2[i]){
            equals = false;
            break;
        }
    }
    return equals;
}

vector<SERVER_COPY> remove_from_server_list(SERVER_COPY server_copy, vector<SERVER_COPY> servers){
    for(int i = 0; i < servers.size(); i++){
        bool ip_equals = str_equals((char*) (server_copy.ip).c_str(), server_copy.ip.size(), (char*) (servers[i].ip).c_str(), servers[i].ip.size());
        bool port_equals = server_copy.PORT == servers[i].PORT;
        if(ip_equals && port_equals){
            //cout << "servers[i].PORT: " << servers[i].PORT << endl;
            servers.erase(servers.begin()+i);
            break;
        }
    }
    return servers;
}

SERVER_COPY update_this_server_info(SERVER_COPY this_server, SERVER_COPY server_copy){
    bool ip_equals = str_equals((char*) (server_copy.ip).c_str(), server_copy.ip.size(), (char*) (this_server.ip).c_str(), this_server.ip.size());
    bool port_equals = server_copy.PORT == this_server.PORT;
    if(ip_equals && port_equals){
        this_server.id = server_copy.id;
    }
    return this_server;
}

SERVER_COPY update_this_server_info(SERVER_COPY this_server, vector<SERVER_COPY> new_servers){
    for(int i = 0; i < new_servers.size(); i++){
        this_server = update_this_server_info(this_server, new_servers[i]);
    }
	return this_server;
}