#include "./client.hpp"
bool Logout = false;
int socketCtrl = -1;
int socketCtrl2 = -1;
char username[256];

vector<string> last_downloads;

mutex mtx_sync_update;

bool wait_for_first_sync = true;

int main(int argc, char *argv[])
{
	bool sync_dir_active = false;

	int sockfd, PORT, newSocket, sockfd_sync;
	socklen_t clilen;
	char buffer[256]; 
	struct sockaddr_in serv_addr, cli_addr;
	struct sigaction sigIntHandler;
	PACKET receivedPkt;
	string message, servAddr;

	sigIntHandler.sa_handler = handle_ctrlc;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	verificaRecebimentoParametros(argc);

	strcpy(username, argv[1]);
	PORT = atoi(argv[3]);
	servAddr = string(argv[2]);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("ERROR opening socket");
		exit(0);
	}

	socketCtrl = sockfd;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	inet_aton(servAddr.c_str(), &serv_addr.sin_addr);
	bzero(&(serv_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR connecting\n");
		exit(0);
	}

	bzero(buffer, 256);

	cout << "write14" << endl;
	sendMessage("", 1, MENSAGEM_LOGIN, 1, username, sockfd); // login message
	cout << "read1" << endl;
	readSocket(&receivedPkt, sockfd);

	//cout<<"read antes if"<< endl;
	//cout<< "receivedPkt.type: " << receivedPkt.type << endl;

	if (receivedPkt.type ==  MENSAGEM_USUARIO_VALIDO)
	{
		pthread_t thr1, thr2, thr3;
		int n1 = 1;
		int n2 = 2;

		while(pthread_create(&thr2, NULL, input, (void *)&n2) != 0){
			cout << "ERROR creating first input thread. retrying..." << endl;
		}

		cout << "type exit to end your session \n"
			 << endl;
		while (true)
		{
			sigaction(SIGINT, &sigIntHandler, NULL);

			if (action.size() > 0 && sync_dir_active)
			{
				mtx_sync_update.lock();
				cout << "action size: " << action.size() << endl;
				for(int i = 0; i < action.size(); i++){
					cout << "action: " << action[i] << " & name: " << name[i] << "\n";
					if(action[i] == FILE_CREATED || action[i] == FILE_MODIFIED){
						if(find(last_downloads.begin(), last_downloads.end(), name[i]) != last_downloads.end()) {
							cout << "skipping downloaded file: " << name[i] << endl;
							continue;
						}
						upload_to_server(sockfd, username, "./sync_dir/"+name[i]);
					}
					else if(action[i] == FILE_DELETED){
						cout << "write1" << endl;
						sendMessage((char *)("./sync_dir/"+name[i]).c_str(), 1, MENSAGEM_DELETAR_NO_SERVIDOR, 1, username, sockfd);
					}
				}
				last_downloads.clear();
				action.clear();
				name.clear();
				mtx_sync_update.unlock();
			}
			if (command.size() > 2)
			{
				if (command == "exit"|| Logout == true )
				{
					cout << "exiting application..." << endl;
					Logout = false;
					break;
				}
				else if (command == "list_client")
				{
					print_file_list("./sync_dir");
				}
				else if (command == "list_server")
				{
					cout << "write2" << endl;
					sendMessage("", 1, MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR, 1, username, sockfd);

					cout << "read2" << endl;
					int result = readSocket(&receivedPkt, sockfd);
					
					while(receivedPkt.type == MENSAGEM_ITEM_LISTA_DE_ARQUIVOS && result > 0){
						cout << receivedPkt._payload;
						cout << "read3" << endl;
						result = readSocket(&receivedPkt, sockfd);
					}
				}
				else if (command.find("upload ") != std::string::npos)
				{
					std::string path = command.substr(command.find("upload ") + 7);
					cout << path << "\n";
					upload_to_server(sockfd, username, path, false);

				}
				else if (command == ("get_sync_dir") && !sync_dir_active)
				{
					sync_dir_active = true;

					//deleta o diretório caso ele exista
					delete_file("./sync_dir");
					//cria o diretório
					create_folder("./sync_dir");

					//cria novo socket para lidar com as atualizações recebidas do servidor pelo cliente

					struct sockaddr_in serv_addr;
					struct sigaction sigIntHandler;

					sigIntHandler.sa_handler = handle_ctrlc;
					sigemptyset(&sigIntHandler.sa_mask);
					sigIntHandler.sa_flags = 0;

					verificaRecebimentoParametros(argc);

					if ((sockfd_sync = socket(AF_INET, SOCK_STREAM, 0)) == -1)
					{
						printf("ERROR opening socket");
						exit(0);
					}

					socketCtrl2 = sockfd_sync;

					serv_addr.sin_family = AF_INET;
					serv_addr.sin_port = htons(PORT);
					inet_aton(servAddr.c_str(), &serv_addr.sin_addr);
					bzero(&(serv_addr.sin_zero), 8);

					if (connect(sockfd_sync, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
					{
						printf("ERROR connecting\n");
						exit(0);
					}

					cout << "sockfd_sync: " << sockfd_sync << endl;

					//informa o servidor que se está recebendo atualizações
					cout << "write3" << endl;
					sendMessage("", 1, GET_SYNC_DIR, 1, username, sockfd_sync);

					//cria nova thread para lidar com atualizações
					int i = pthread_create(&thr1, NULL, handle_updates, &sockfd_sync) != 0;
					while(i != 0){
						i = pthread_create(&thr1, NULL, handle_updates, &sockfd_sync);
						cout << "ERROR creating handle_updates thread. retrying..." << endl;
						cout << "ERROR number: " << i << endl;
					}

					while(wait_for_first_sync){
						sleep(1);
					}

					//cria nova thread para lidar com modificações na pasta sync_dir
					while(pthread_create(&thr3, NULL, folderchecker, (void *)&n1) != 0){
						cout << "ERROR creating folderchecker thread. retrying..." << endl;
					}
				}
				else if (command.find("download ") != std::string::npos)
				{
					std::string path = command.substr(command.find("download ") + 9);
					download_file_from_server(sockfd,username,path);
				}
				else if (command.find("delete ") != std::string::npos)
				{
					std::string path = command.substr(command.find("delete ") + 7);
					cout << "write4" << endl;
					sendMessage((char *)path.c_str(), 1, MENSAGEM_DELETAR_NO_SERVIDOR, 1, username, sockfd);
				}
				command = "";
			}
		}
		cout << "write5" << endl;
		sendMessage("", 1, MENSAGEM_LOGOUT, 1, username, sockfd); // logout message
	}
	else
	{
		cout << endl << receivedPkt._payload << endl;
	}

	close(sockfd_sync);
	close(sockfd);
	return 0;
}

void verificaRecebimentoParametros(int argc)
{
	if (argc < 3)
	{
		cout << "Faltam parametros" << endl;
		exit(0);
	}
}

int upload_to_server(int sock, char username[],std::string file_path, bool sync)
{
	char buffer[256];
	PACKET pktreceived;

	ifstream file;
	file.open(file_path, ios_base::binary);
	if (!file.is_open())
	{
		cout << "falha ao abrir" << "\n" << endl;
		return 0;
		// mensagem erro
	}
	else
	{	
		file.seekg(0, file.end);
		int file_size = file.tellg();
		cout << "file_size: " << file_size << endl;
		file.clear();
		file.seekg(0);

		int max_fragments = file_size/256;

		if(file_size % 256 != 0){
			max_fragments++;
		}

		cout << "max_fragments: " << max_fragments << endl;
		
		cout << "write6" << endl;
		sendMessage((char*)file_path.c_str(), 1, MENSAGEM_ENVIO_NOME_ARQUIVO, max_fragments, username, sock);
		cout << "read51" << endl;
		readSocket(&pktreceived, sock);
		if(pktreceived.type != ACK){
			file.close();
			return 1;
		}
		int counter=0;
		for (int i=0;i< file_size;i+=((sizeof(buffer)))) // to read file
		{
			memset(buffer, 0, 256);
			file.read(buffer,sizeof(buffer));
			if(sync){
				cout << "write7" << endl;
				sendMessage(buffer, i/256 , MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC, max_fragments, username, sock);
			}else{
				cout << "write8" << endl;
				sendMessage(buffer, i/256 , MENSAGEM_ENVIO_PARTE_ARQUIVO, max_fragments, username, sock);
			}
			counter++;
			cout << "counter: " << counter << endl;
			if(counter % 200 == 199){
				cout << "read30" << endl;
				readSocket(&pktreceived,sock);
			}
		}
		file.close();

		cout << "write9" << endl;
        sendMessage(buffer, 1, MENSAGEM_ARQUIVO_LIDO, max_fragments, username, sock);
		cout << "read50" << endl;
		readSocket(&pktreceived, sock);

		cout << "arquivo lido."
			 << "\n"
			 << endl;
		return 1;
	}
}
int download_file_from_server(int sock,char username[], std::string file_path)
{	
	PACKET pkt;
	cout << "write10" << endl;
	sendMessage((char *)file_path.c_str(), 1, MENSAGEM_DOWNLOAD_NO_SERVIDOR, 1, username, sock);
	cout << "read6" << endl;
	readSocket(&pkt, sock);
	if(pkt.type==MENSAGEM_FALHA_ENVIO){
		cout<<"Arquivo nao existe no servidor\n";
		return 0;
	}
	string receivedFilePath;
	receivedFilePath = string(pkt._payload);
	receivedFilePath = receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
	last_downloads.push_back(receivedFilePath);
	string directory = ".";
	directory = directory + "/" + receivedFilePath;
	cout << directory;
	ofstream file_download;
	file_download.open(directory, ios_base::binary);
	int size = pkt.total_size;
	int received_fragments = 0;
	vector<vector<char>> fragments(size);
	while (received_fragments != size)
	{
		memset(pkt._payload, 0, 256);
		cout << "read7" << endl;
		readSocket(&pkt, sock);
		char buffer[256];
		vector<char> bufferconvert(256);

		memset(buffer, 0, 256);

		memcpy(buffer, pkt._payload, 256);

		printf("%d\n", received_fragments);
		for (int i = 0; i < bufferconvert.size(); i++)
		{
			bufferconvert[i] = buffer[i];
		}
		if (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC)
		{
			if(received_fragments >= fragments.size()){
				cout << "1: received_fragments >= fragments.size()" << endl;
			}
			fragments.at(received_fragments)=bufferconvert;
			received_fragments++;
		}
		cout << "received_fragments: " << received_fragments << " & size: " << size << endl;
	}
	for (int i = 0; i < fragments.size(); i++)
	{
		for (int j = 0; j < fragments.at(i).size(); j++)
		{
			char *frag = &(fragments.at(i).at(j));
			//printf("%x ", (unsigned char)fragments.at(i).at(j));
			file_download.write(frag, sizeof(char));
		}
	}
	file_download.close();
	cout << "read23" << endl;
	readSocket(&pkt, sock);
	if(pkt.type != MENSAGEM_ARQUIVO_LIDO){
		cout << "expected msg type: " << MENSAGEM_ARQUIVO_LIDO << endl;
		cout << "received msg type: " << pkt.type << endl;
		cout << "error on download_file_from_server()" << endl;
	}
	return 1;
}
void handle_ctrlc(int s){
	PACKET Pkt;

	Logout = true;
	cout<<endl<<"Caught signal"<<endl;
	cout << "write12" << endl;
	sendMessage("", 1, MENSAGEM_LOGOUT, 1, username, socketCtrl); // logout message
	
	cout << endl << Pkt._payload << endl;

	close(socketCtrl);
	close(socketCtrl2);

	exit(0);
}

void *handle_updates(void *arg)
{
    int sockfd = *(int *)arg;

	PACKET pkt;

	ofstream file_client;
	int size = 0;
	int received_fragments=0;
    vector<vector<char>> fragments(10);
	
	while(true){
		cout << "read10" << endl;
		readSocket(&pkt, sockfd);
		cout << "pkt.type: " << pkt.type << ". ";
		if(pkt.type == MENSAGEM_DELETAR_NOS_CLIENTES){
			string toRemoveFilePath;

            toRemoveFilePath = string(pkt._payload);
            toRemoveFilePath = toRemoveFilePath.substr(toRemoveFilePath.find_last_of("/") + 1);
            string file_path = "./";
            file_path = file_path + "sync_dir" + "/" + toRemoveFilePath;

			mtx_sync_update.lock();
            int result = delete_file(file_path);
			mtx_sync_update.unlock();

			if(result == -1){
				cout << "file path:" << endl;
				cout << file_path << endl;
				cout << "could not delete file" << endl;
			}
		}
		if (pkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO)
        {
            string receivedFilePath;

            receivedFilePath = string(pkt._payload);
            receivedFilePath = receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
			last_downloads.push_back(receivedFilePath);
            string directory = "./";
            directory = directory + "sync_dir" + "/" + receivedFilePath;
			mtx_sync_update.lock();
            file_client.open(directory, ios_base::binary);
            size = pkt.total_size;

            cout << directory << "\n" << endl;

            fragments.clear();
            fragments.resize(size);
            received_fragments = 0;

			cout << "write43" << endl;
        	sendMessage("", 1, ACK, 1, username, sockfd);

			if(size == 0){
				mtx_sync_update.unlock();
			}
        }
        if(pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC)
        {
            char buffer [256];
            vector<char> bufferconvert(256);

            memset(buffer, 0, 256);

            memcpy(buffer,pkt._payload, 256);

            //printf("%d",received_fragments);

            for(int i = 0; i < bufferconvert.size(); i++){
                bufferconvert[i] = buffer[i];
            }
            
			cout << "received_fragments: " << received_fragments;
			cout << ". pkt.seqn: " << pkt.seqn;
			cout << ". fragments.size(): " << fragments.size() << endl;
			fragments.at(pkt.seqn)=bufferconvert;
			received_fragments++;

			if(received_fragments % 200 == 199){
				cout << "write13" << endl;
                sendMessage("", 1, ACK, 1, username, sockfd);
            }

			cout << "received_fragments: " << received_fragments << endl;

            if(received_fragments == size)
            {
				cout << "escrevendo" << endl;
                for (int i =0 ;i<fragments.size();i++){
                    for(int j=0;j<fragments.at(i).size();j++){
                        char* frag = &(fragments.at(i).at(j));
                        //printf("%x ",(unsigned char)fragments.at(i).at(j));
                        file_client.write(frag, sizeof(char));
                    }
                }
                file_client.close();
				mtx_sync_update.unlock();
            }
        }
		if(pkt.type == MENSAGEM_ARQUIVO_LIDO){
			sendMessage("", 1, ACK, 1, username, sockfd);
		}
		if(pkt.type == FIRST_SYNC_END && wait_for_first_sync){
			wait_for_first_sync = false;
		}
	}
}