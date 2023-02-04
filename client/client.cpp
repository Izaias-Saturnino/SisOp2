#include "./client.hpp"
bool Logout = false;

int main(int argc, char *argv[])
{
	int sockfd, PORT, newSocket;
	socklen_t clilen;
	char buffer[256], username[256];
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

	sendMessage("", 1, MENSAGEM_LOGIN, 1, username, sockfd); // login message

	readSocket(&receivedPkt, sockfd);

	if (receivedPkt.type ==  MENSAGEM_USUARIO_VALIDO)
	{
		pthread_t thr1, thr2;
		int n1 = 1;
		int n2 = 2;
		//TO DO: somente criar se comando get sync dir for ativado pthread_create(&thr1, NULL, folderchecker, (void *)&n1); 
		pthread_create(&thr2, NULL, input, (void *)&n2);
		cout << "type exit to end your session \n"
			 << endl;
		while (true)
		{
			sigaction(SIGINT, &sigIntHandler, NULL);

			// TO DO: Command é global? precisa de mutex?
			if (action != 0)
			{
				std::cout << action << " " << name << "\n";
				action = 0;
			}
			if (command_complete)
			{		
				if (command == "exit" || Logout == true)
				{
					break;
				}
				if (command == "list_client")
				{
					print_file_list("./sync_dir");
				}
				if (command == "list_server")
				{
					sendMessage("", 1, MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR, 1, username, sockfd);
					readSocket(&receivedPkt, sockfd);

					while(receivedPkt.type == MENSAGEM_ITEM_LISTA_DE_ARQUIVOS){
						cout<<receivedPkt._payload;
						readSocket(&receivedPkt, sockfd);
					}
				}
				if (command.find("upload ") != std::string::npos)
				{
					std::string path = command.substr(command.find("upload ") + 7);
					cout << path;
					upload_file_client(sockfd, username, path);

				}
				if (command.find("download ") != std::string::npos)
				{
					std::string path = command.substr(command.find("download ") + 9);
					download_file_client(sockfd,username,path);
				}
				command = "";
				pthread_create(&thr2, NULL, input, (void *)&n2);
			}
		}

		sendMessage("", 1, MENSAGEM_LOGOUT, 1, username, sockfd); // logout message

		readSocket(&receivedPkt, sockfd);

		cout << endl << receivedPkt._payload << endl;
	}
	else
	{
		cout <<endl << receivedPkt._payload << endl;
	}

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

int upload_file_client(int sock, char username[],std::string file_path)
{
	char buffer[256];


	ifstream file;
	file.open(file_path, ios_base::binary);
	if (!file.is_open())
	{
		cout << " falha ao abrir"
			 << "\n"
			 << endl;
		return 0;
		// mensagem erro
	}
	else
	{	file.seekg(0, file.end);
		int file_size = file.tellg();
		cout << file_size <<  "\n";
		file.clear();
		file.seekg(0);
		
		sendMessage((char*)file_path.c_str(), 1, MENSAGEM_ENVIO_NOME_ARQUIVO, std::ceil(file_size/256), username, sock);
		for (int i=0;i< file_size;i+=((sizeof(buffer)))) // to read file
		{	
			memset(buffer, 0, 256);
			file.read(buffer,sizeof(buffer));
			sendMessage(buffer, i/256 , MENSAGEM_ENVIO_PARTE_ARQUIVO, 4, username, sock);
		}
		file.close();
		sendMessage(buffer, 1, MENSAGEM_ARQUIVO_LIDO, 4, username, sock);
		cout << " arquivo lido"
			 << "\n"
			 << endl;
		return 1;
	}
}
int download_file_client(int sock,char username[], std::string file_path)
{	
	PACKET pkt;
	sendMessage((char *)file_path.c_str(), 1, MENSAGEM_DOWNLOAD_NO_SERVIDOR, 1, username, sock);
	readSocket(&pkt, sock);
	if(pkt.type==MENSAGEM_FALHA_ENVIO){
		cout<<"Arquivo nao existe no servidor\n";
		return 0;
	}
	string receivedFilePath;
	receivedFilePath = string(pkt._payload);
	receivedFilePath = receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
	string directory = "./sync_dir";
	directory = directory + "/" + receivedFilePath;
	cout << directory;
	ofstream file_download;
	file_download.open(directory, ios_base::binary);
	int size = pkt.total_size;
	int received_fragments = 0;
	vector<vector<char>> fragments(size+1);
	while (received_fragments != size)
	{
		memset(pkt._payload, 0, 256);
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
		if (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO)
		{
			received_fragments++;
			fragments.at(pkt.seqn) = bufferconvert;
		}
	}
	for (int i = 0; i < fragments.size(); i++)
	{
		for (int j = 0; j < fragments.at(i).size(); j++)
		{
			char *frag = &(fragments.at(i).at(j));
			printf("%x ", (unsigned char)fragments.at(i).at(j));
			file_download.write(frag, sizeof(char));
		}
	}
	file_download.close();
	return 1;
}
void handle_ctrlc(int s){
	Logout = true;
	cout<<"Caught signal"<<endl;
}