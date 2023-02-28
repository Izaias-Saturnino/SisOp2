#include "./client.hpp"
bool Logout = false;
int socketCtrl = -1;
int socketCtrl2 = -1;

vector<string> latest_downloads = {};

bool wait_for_first_sync = true;

int main(int argc, char *argv[])
{
	bool sync_dir_active = false;

	int sockfd_sync;
	socklen_t clilen;
	struct sigaction sigIntHandler;
	PACKET receivedPkt;

	sigIntHandler.sa_handler = handle_ctrlc;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	verificaRecebimentoParametros(argc);

	char* server_ip = argv[2];
	hostent *server_host = gethostbyname(server_ip);
	int PORT = atoi(argv[3]);
	int sockfd = connect_to_server(*server_host, PORT);
	
	if (sockfd == -1)
	{
		exit(0);
	}

	socketCtrl = sockfd;

	char username[BUFFER_SIZE];
	strcpy(username, argv[1]);

	cout << "send login" << endl;
	sendMessage(username, MENSAGEM_LOGIN, sockfd); // login message
	cout << "read login" << endl;
	readSocket(&receivedPkt, sockfd);

	if (receivedPkt.type == MENSAGEM_USUARIO_VALIDO)
	{
		pthread_t thr1, thr2, thr3;
		int n1 = 1;
		int n2 = 2;

		cout << "creating input thread" << endl;
		create_thread(&thr2, NULL, input, (void *)&n2);

		cout << "type exit to end your session \n"
			 << endl;
		while (true)
		{
			//cout<<"size of packet: " << sizeof(PACKET) << endl;
			sigaction(SIGINT, &sigIntHandler, NULL);

			if (action.size() > 0 && sync_dir_active)
			{
				mtx_file_manipulation.lock();
				cout << "action size: " << action.size() << endl;
				for (int i = 0; i < action.size(); i++)
				{
					cout << "action: " << action[i] << " & name: " << name[i] << "\n";
					if (action[i] == FILE_CREATED || action[i] == FILE_MODIFIED)
					{
						if (find(latest_downloads.begin(), latest_downloads.end(), name[i]) != latest_downloads.end())
						{
							continue;
						}
						cout << "write111" << endl;
						sendMessage("", MENSAGEM_ENVIO_SYNC, sockfd);
						upload_to_server(sockfd, "./sync_dir/" + name[i]);
					}
					if (action[i] == FILE_DELETED)
					{
						cout << "write1" << endl;
						sendMessage((char *)("./sync_dir/" + name[i]).c_str(), MENSAGEM_DELETAR_NO_SERVIDOR, sockfd);
					}
				}
				latest_downloads.clear();
				action.clear();
				name.clear();
				mtx_file_manipulation.unlock();
			}
			//cout << "command before thread:" << command << endl;
			if (command_complete)
			{
				cout << "command inside main thread:" << command << endl;
				if (command == "exit" || Logout == true)
				{
					cout << "exiting application..." << endl;
					Logout = false;
					break;
				}
				else if (command == "list_client")
				{
					print_file_list("./sync_dir");
					cout << "listing ended" << endl;
				}
				else if (command == "list_server")
				{
					cout << "sending list request" << endl;
					sendMessage("", MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR, sockfd);

					cout << "read2" << endl;
					int result = readSocket(&receivedPkt, sockfd);

					while (receivedPkt.type == MENSAGEM_ITEM_LISTA_DE_ARQUIVOS && result > 0)
					{
						cout << receivedPkt._payload;
						cout << "read3" << endl;
						result = readSocket(&receivedPkt, sockfd);
					}
					cout << "listing ended" << endl;
				}
				else if (command.find("upload ") == 0)
				{
					cout << "uploading file" << endl;
					string path = command.substr(command.find("upload ") + 7);
					cout << "file path: " << path << endl;
					cout << path << "\n";
					cout << "write112" << endl;
					mtx_file_manipulation.lock();
					upload_to_server(sockfd, path);
					mtx_file_manipulation.unlock();
					cout << "file uploaded" << endl;
				}
				else if (command == "get_sync_dir" && !sync_dir_active)
				{
					sync_dir_active = true;

					mtx_file_manipulation.lock();
					// deleta o diretório caso ele exista
					delete_file("./sync_dir");
					// cria o diretório
					create_folder("./sync_dir");
					mtx_file_manipulation.unlock();

					// cria novo socket para lidar com as atualizações recebidas do servidor pelo cliente
					sockfd_sync = connect_to_server(*server_host);
					if(sockfd_sync == -1){
						cout << "ERROR creating sync socket" << endl;
						command = "";
						command_complete = false;
					}
					cout << "sockfd_sync: " << sockfd_sync << endl;

					// informa o servidor que se está recebendo atualizações
					cout << "write3" << endl;
					sendMessage(username, GET_SYNC_DIR, sockfd_sync);

					// cria nova thread para lidar com atualizações
					cout << "creating thread to handle_updates" << endl;
					create_thread(&thr1, NULL, handle_updates, &sockfd_sync);

					while (wait_for_first_sync)
					{
						sleep(1);
					}

					// cria nova thread para lidar com modificações na pasta sync_dir
					cout << "creating folderchecker thread" << endl;
					create_thread(&thr3, NULL, folderchecker, (void *)&n1);
					cout << "sync_dir active" << endl;
				}
				else if (command.find("download ") == 0)
				{
					string path = command.substr(command.find("download ") + 9);
					cout << "downloading file" << endl;
					cout << "file path: " << path << endl;
					download_file_from_server(sockfd, path);
					cout << "file downloaded" << endl;
				}
				else if (command.find("delete ") == 0)
				{
					string path = command.substr(command.find("delete ") + 7);
					cout << "sending delete request" << endl;
					cout << "file path: " << path << endl;
					cout << "write4" << endl;
					sendMessage((char *)path.c_str(), MENSAGEM_DELETAR_NO_SERVIDOR, sockfd);
					cout << "file deleted" << endl;
				}
				command = "";
				command_complete = false;
			}
		}
		cout << "write5" << endl;
		sendMessage("", MENSAGEM_LOGOUT, sockfd); // logout message
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
		cout << "parameters missing" << endl;
		exit(0);
	}
}

void upload_to_server(int sock, string file_path)
{
	sendFile(sock, file_path);
}

int download_file_from_server(int sock, string file_path)
{
	PACKET pkt;
	cout << "download file from server function" << endl;
	sendMessage((char *)file_path.c_str(), MENSAGEM_DOWNLOAD_FROM_SERVER, sock);

	cout << "read download name" << endl;
	readSocket(&pkt, sock);
	if(pkt.type == MENSAGEM_FALHA_ENVIO){
		cout << "file does not exist on server\n";
		return 1;
	}

	string file_name = getFileName(file_path);
	string directory = "./" + file_name;
	mtx_file_manipulation.lock();
	receiveFile(sock, directory, &pkt);
	latest_downloads.push_back(file_name);
	mtx_file_manipulation.unlock();
	return 0;
}

void handle_ctrlc(int s)
{
	PACKET Pkt;

	Logout = true;
	cout << endl << "Caught signal" << endl;
	//cout << "write12" << endl;
	//sendMessage("", MENSAGEM_LOGOUT, socketCtrl); // logout message

	//cout << endl << Pkt._payload << endl;

	//close(socketCtrl);
	//close(socketCtrl2);
}

void *handle_updates(void *arg)
{
	int sockfd = *(int *)arg;

	PACKET pkt;

	ofstream file_client;
	int received_fragments = 0;
	vector<vector<char>> fragments = {};
	string receivedFilePath;
	uint32_t remainder_file_size = 0;

	while (true)
	{
		cout << "handleUpdates function" << endl;
		readSocket(&pkt, sockfd);
		cout << "pkt.type: " << pkt.type << ". ";
		if (pkt.type == MENSAGEM_DELETAR_NOS_CLIENTES)
		{
			string file_name = getFileName(string(pkt._payload));

			string file_path = "./";
			file_path = file_path + "sync_dir" + "/" + file_name;

			mtx_file_manipulation.lock();
			int result = delete_file(file_path);
			mtx_file_manipulation.unlock();

			if (result == -1)
			{
				cout << "file path:" << endl << file_path << endl;
				cout << "could not delete file" << endl;
			}
		}
		if (pkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO)
		{
			string file_name = getFileName(string(pkt._payload));
			string directory = "./sync_dir/" + file_name;
			mtx_file_manipulation.lock();
			receiveFile(sockfd, directory, &pkt);
			latest_downloads.push_back(file_name);
			mtx_file_manipulation.unlock();
		}
		if (pkt.type == FIRST_SYNC_END && wait_for_first_sync)
		{
			latest_downloads.clear();
			wait_for_first_sync = false;
		}
	}
}