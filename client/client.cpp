#include "./client.hpp"

bool Logout = false;
int sockfd;
char username[256];

int main(int argc, char *argv[])
{
	int PORT, newSocket;
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
				if (command == "exit"|| Logout == true)
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
				if (command == "upload")
				{
					upload_file_client();

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

int upload_file_client()
{
	string file_path;
	std::string buffer;

	cout << "Informe o caminho do arquivo a ser enviado"
		 << "\n";
	cin >> file_path;
	ifstream file;
	file.open(file_path);
	if (!file.is_open())
	{
		cout << " falha ao abrir"
			 << "\n"
			 << endl;
		return 0;
		// mensagem erro
	}
	else
	{
		sendMessage(file_path, 1, MENSAGEM_ENVIO_NOME_ARQUIVO, 4, username, sockfd);

		while (getline(file, buffer)) // to read file
		{
			// function used to read the contents of file
			sendMessage(buffer, 1, MENSAGEM_ENVIO_PARTE_ARQUIVO, 4, username, sockfd);
			cout << buffer << "\n"
				 << endl;
			buffer.clear();
		}
		file.close();
		sendMessage(buffer, 1, MENSAGEM_ARQUIVO_LIDO, 4, username, sockfd);
		cout << " arquivo lido"
			 << "\n"
			 << endl;
		return 1;
	}
}

void handle_ctrlc(int s){
	PACKET Pkt;

	Logout = true;
	cout<<endl<<"Caught signal"<<endl;

	sendMessage("", 1, MENSAGEM_LOGOUT, 1, username, sockfd); // logout message
	readSocket(&Pkt, sockfd);
	cout << endl << Pkt._payload << endl;
	
	close(sockfd);
	exit(0);
}