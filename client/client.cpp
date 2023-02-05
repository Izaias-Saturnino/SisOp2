#include "./client.hpp"
bool Logout = false;
int socketCtrl;
char username[256];

int main(int argc, char *argv[])
{
	bool sync_dir_active = false;

	int sockfd, PORT, newSocket;
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

	sendMessage("", 1, MENSAGEM_LOGIN, 1, username, sockfd); // login message

	readSocket(&receivedPkt, sockfd);

	cout<<"read antes if"<< endl;
	cout<<receivedPkt.type<< endl;

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
				if (command == "exit"|| Logout == true )
				{
					Logout = false;					
					break;
				}
				else if (command == "list_client")
				{
					print_file_list("./sync_dir");
				}
				else if (command == "list_server")
				{
					sendMessage("", 1, MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR, 1, username, sockfd);
					readSocket(&receivedPkt, sockfd);

					while(receivedPkt.type == MENSAGEM_ITEM_LISTA_DE_ARQUIVOS){
						cout<<receivedPkt._payload;
						readSocket(&receivedPkt, sockfd);
					}
				}
				else if (command.find("upload ") != std::string::npos)
				{
					std::string path = command.substr(command.find("upload ") + 7);
					cout << path << "\n";
					upload_file_client(sockfd, username, path);

				}
				else if (command == ("get_sync_dir") && !sync_dir_active)
				{
					sync_dir_active = true;

					//criar diretório se não existe
					create_folder("./sync_dir");

					//cria novo socket para lidar com as atualizações recebidas do servidor pelo cliente

					int sockfd_sync;
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

					serv_addr.sin_family = AF_INET;
					serv_addr.sin_port = htons(PORT);
					inet_aton(servAddr.c_str(), &serv_addr.sin_addr);
					bzero(&(serv_addr.sin_zero), 8);

					if (connect(sockfd_sync, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
					{
						printf("ERROR connecting\n");
						exit(0);
					}

					//cria nova thread para lidar com atualizações
					pthread_create(&thr1, NULL, handle_updates, &sockfd_sync);

					//informa o servidor que se está recebendo atualizações
					sendMessage("", 1, GET_SYNC_DIR, 1, username, sockfd_sync);
				}
				else if (command.find("download ") != std::string::npos)
				{
					std::string path = command.substr(command.find("download ") + 9);
					download_file_client(sockfd,username,path);
				}
				else if (command.find("delete ") != std::string::npos)
				{
					std::string path = command.substr(command.find("delete ") + 7);
					sendMessage((char *)path.c_str(), 1, MENSAGEM_DELETAR_NO_SERVIDOR, 1, username, sockfd);
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
	while (received_fragments != size+1)
	{
		memset(pkt._payload, 0, 256);
		readSocket(&pkt, sock);
		char buffer[256];
		vector<char> bufferconvert(256);

		memset(buffer, 0, 256);

		memcpy(buffer, pkt._payload, 256);

		//printf("%d\n", received_fragments);
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
			//printf("%x ", (unsigned char)fragments.at(i).at(j));
			file_download.write(frag, sizeof(char));
		}
	}
	file_download.close();
	return 1;
}
void handle_ctrlc(int s){
	PACKET Pkt;

	Logout = true;
	cout<<endl<<"Caught signal"<<endl;
	sendMessage("", 1, MENSAGEM_LOGOUT, 1, username, socketCtrl); // logout message
	readSocket(&Pkt, socketCtrl);
	
	cout << endl << Pkt._payload << endl;

	close(socketCtrl);

	exit(0);
}

void *handle_updates(void *arg)
{
    int sockfd = *(int *)arg;

	PACKET receivedPkt;
	//ler do socket e verificar se tem mensagem ou não
	//enquanto mensagens existirem, tratar as mensagens
	//quando não houver mais mensagens terminar o laço
	while(true){
		readSocket(&receivedPkt, sockfd);
		if(receivedPkt.type == MENSAGEM_DELETAR_NOS_CLIENTES){
			string toRemoveFilePath;

            toRemoveFilePath = string(receivedPkt._payload);
            toRemoveFilePath = toRemoveFilePath.substr(toRemoveFilePath.find_last_of("/") + 1);
            string file_path = "./sync_dir";
            file_path = file_path + "/" + toRemoveFilePath;

            int result = delete_file(file_path);

			if(result == -1){
				cout << "file path:" << endl;
				cout << file_path << endl;
				cout << "could not delete file" << endl;
			}
		}
		if(receivedPkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO){

		}
		if(receivedPkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || receivedPkt.type == MENSAGEM_ARQUIVO_LIDO){

		}
		if(receivedPkt.type == MENSAGEM_PEDIDO_LISTA_ARQUIVOS_CLIENTE){
			
		}
	}
}