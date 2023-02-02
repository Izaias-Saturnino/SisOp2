#include "./client.hpp"
#include <mutex>
int upload_file_client(int sock, char username[]);
int main(int argc, char *argv[])
{
	int sockfd, PORT, newSocket;
	socklen_t clilen;
	char buffer[256], username[256];
	struct sockaddr_in serv_addr, cli_addr;
	PACKET receivedPkt;
	string message, servAddr;
	bool Logout = false;

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

	sendMessage("", 1, 1, 1, username, sockfd); // login message

	readSocket(&receivedPkt, sockfd);

	if (strcmp(receivedPkt._payload, "OK") == 0)
	{
		pthread_t thr1, thr2;
		int n1 = 1;
		int n2 = 2;
		// somente criar se comando get sync dir for ativado pthread_create(&thr1, NULL, folderchecker, (void *)&n1);
		pthread_create(&thr2, NULL, input, (void *)&n2);
		cout << "type exit to end your session \n"
			 << endl;
		while (true)
		{
			if (action != 0)
			{
				std::cout << action << " " << name << "\n";
				action = 0;
			}
			if (command != "")
			{
				if (command == "exit")
				{
					break;
				}
				if (command == "list_client")
				{
					print_file_list("./sync_dir");
				}
				if (command == "upload")
				{
					upload_file_client(sockfd, username);

					sendMessage("", 1, 5, 1, username, sockfd); // requisição para enviar arquivo
				}
				command = "";
				pthread_create(&thr2, NULL, input, (void *)&n2);
			}
		}

		sendMessage("", 1, 2, 1, username, sockfd); // logout message

		readSocket(&receivedPkt, sockfd);

		cout << receivedPkt._payload << endl;
	}
	else
	{
		cout << receivedPkt._payload << endl;
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

int upload_file_client(int sock, char username[])
{
	string file_path;
	char *buffer;

	cout << "Informe o caminho do arquivo a ser enviado"<<"\n";
	cin >> file_path;
	ifstream file;
	file.open(file_path);
	if (!file.is_open())
	{
		cout << " falha ao abrir" << "\n" << endl;
		return 0;
		// mensagem erro
	}
	else
	{	
		sendMessage(file_path, 1, 10, 4, username, sock);
		while (!file.eof()) // to read file
		{
			// function used to read the contents of file
			file.read(buffer, sizeof(buffer));
			sendMessage(buffer, 1, 11, 4, username, sock);
		}
		file.close();
		return 1;
	}
}
