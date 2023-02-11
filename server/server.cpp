#include "./server.hpp"

#define PORT 4000

LoginManager *loginManager = new LoginManager();
char user[BUFFER_SIZE];
int newSockfd,connectionSocket;
bool END = false;
vector<int> connections;

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;
    struct sigaction sigIntHandler;
    pthread_t clientThread;
    socklen_t clilen;
    bool usuarioValido;
    PACKET pkt;

    sigIntHandler.sa_handler = handle_ctrlc;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

    verificaRecebimentoIP(argc, argv);

    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        imprimeServerError();
    }

    if ((connectionSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) /// Verifica IP valido
        printf("ERROR opening socket\n");

    connections.push_back(connectionSocket);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(connectionSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(0);
    }
    else
    {
        printf("server started\n");
        while (true)
        {

            /*listen to clients*/
            listen(connectionSocket, 5);
            clilen = sizeof(struct sockaddr_in);

            newSockfd = accept(connectionSocket, (struct sockaddr *)&cli_addr, &clilen);

            sigaction(SIGINT, &sigIntHandler, NULL);
            if(END){
                break;
            }

            if (newSockfd == -1){
                cout << "ERROR on accept" << endl;
            }
            else
            {
                connections.push_back(newSockfd);

                /*inicio login*/
                cout << "read0" << endl;
                readSocket(&pkt, newSockfd);
                strcpy(user, pkt.user);
                if(pkt.type == MENSAGEM_LOGIN){
                    usuarioValido = loginManager->login(newSockfd, user);

                    if (usuarioValido)
                    {
                        string path = "./" + string(user);
                        cout << path << "\n";
                        if (!filesystem::is_directory(path))
                        {
                            cout << path << "\n";
                            create_folder(path);
                        }
                        cout << "write1" << endl;
                        sendMessage("OK", 1, MENSAGEM_USUARIO_VALIDO, 1, user, newSockfd); // Mensagem de usuario Valido

                        while(pthread_create(&clientThread, NULL, ThreadClient, &newSockfd) != 0){ // CUIDADO: newSocket e não socket
                            cout << "ERROR creating first input thread. retrying..." << endl;
                        }
                    }
                    else
                    {
                        cout << "write2" << endl;
                        sendMessage("Excedido numero de sessoes", 1, MENSAGEM_USUARIO_INVALIDO, 1, user, newSockfd); // Mensagem de usuario invalido
                    }
                }
                else if(pkt.type == GET_SYNC_DIR){
                    //baixar todos os arquivos do syncdir do servidor
                    vector<string> file_paths = get_file_list("./" + string(user));

                    for(int i = 0; i < file_paths.size(); i++){
                        send_file_to_client(newSockfd, user, file_paths[i]);
                    }

                    cout << "write3" << endl;
                    sendMessage("", 1, FIRST_SYNC_END, 1, user, newSockfd);
            
                    //salvar o socket que pediu atualizações de sync dir
                    loginManager->activate_sync_dir(user, newSockfd);

                    cout << "active sync dir confirmed" << endl;
                }
            }
        }
    }

    cout << endl << "closing connections" << endl;
    close_connections();

    return 0;
}

void verificaRecebimentoIP(int argc, char *argv[])
{
    if (argc < 2)
    { // Verifica se recebeu o IP como parametro
        fprintf(stderr, "usage %s hostname\n", argv[0]);
        exit(0);
    }
}

void imprimeServerError(void)
{
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
}

void *ThreadClient(void *arg)
{
    int sockfd = *(int *)arg;
    PACKET pkt;
    char resposta[40];
    char user[BUFFER_SIZE];
    ofstream file_server;
    uint32_t size=0;
    int received_fragments=0;
    vector<vector<char>> fragments = {};
    string directory;

    bool last_message_sync = false;

    uint32_t remainder_file_size = 0;

    while (true)
    {
        memset(pkt._payload, 0, BUFFER_SIZE);
        cout << "read1" << endl;
        readSocket(&pkt, sockfd);
        strcpy(user, pkt.user);
        cout << "pkt.type: " << pkt.type << ". ";
 
        if (pkt.type == MENSAGEM_LOGOUT)
        {
            loginManager->Logout(user, sockfd, resposta);
            break;
        }
        if (pkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO)
        {
            remainder_file_size = pkt.file_byte_size % BUFFER_SIZE;
            cout << "remainder_file_size: " << remainder_file_size << endl;

            string receivedFilePath;

            receivedFilePath = string(pkt._payload);
            receivedFilePath = receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
            directory = "./";
            directory = directory + pkt.user + "/" + receivedFilePath;
            file_server.open(directory, ios_base::binary);
            size = pkt.file_byte_size % BUFFER_SIZE;

            cout << directory << "\n"
                 << endl;

            fragments.clear();
            received_fragments = 0;

            if(size == 0){
                file_server.close();

                vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

                cout << "directory: " << directory << endl;

                for(int i = 0; i < sync_dir_sockets.size(); i++){
                    cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << endl;
                    send_file_to_client(sync_dir_sockets[i],user,directory);
                }
            }
        }
        if(pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC){
            last_message_sync = true;
        }
        if(pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC)
        {
            char buffer [BUFFER_SIZE];
            vector<char> bufferconvert(BUFFER_SIZE);

            memset(buffer, 0, BUFFER_SIZE);

            memcpy(buffer,pkt._payload, BUFFER_SIZE);

            //printf("%d",received_fragments);

            for(int i = 0; i < bufferconvert.size(); i++){
                bufferconvert[i] = buffer[i];
            }
            
            cout << "received_fragments: " << received_fragments;
            cout << ". pkt.file_byte_size: " << pkt.file_byte_size;
            cout << ". fragments.size(): " << fragments.size() << endl;
            fragments.push_back(bufferconvert);
            received_fragments++;

            /*if(received_fragments % 300 == 299){
                cout << "write26" << endl;
			    sendMessage("", 1, ACK, 1, user, sockfd);
		    }*/
            cout << "received_fragments: " << received_fragments << " & size: " << size << endl;
        }
        if(pkt.type == MENSAGEM_ARQUIVO_LIDO){
            cout << "reasembling file" << endl;
            for (int i = 0; i < fragments.size(); i++){
				for (int j = 0; j < fragments.at(i).size(); j++)
				{
                    if(i == fragments.size() - 1){
                        if(j == remainder_file_size){
                            cout << "remainder break" << endl;
                            cout << "remainder_file_size: " << remainder_file_size << endl;
                            break;
                        }
                    }
					char *frag = &(fragments.at(i).at(j));
					//printf("%x ", (unsigned char)fragments.at(i).at(j));
					file_server.write(frag, sizeof(char));
				}
			}
            file_server.close();

            vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

            cout << "directory: " << directory << endl;

            cout << "sync_dir_sockets.size(): " << sync_dir_sockets.size() << endl;

            for(int i = 0; i < sync_dir_sockets.size(); i++){
                if(last_message_sync){
                    if(sync_dir_sockets[i] == loginManager->get_sender_sync_sock(sockfd)){
                        cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << " não recebe o arquivo." << endl;
                        continue;
                    }
                }
                cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << endl;
                send_file_to_client(sync_dir_sockets[i],user,directory);
            }
            last_message_sync = false;
        }
        if (pkt.type == MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR)
        {
            vector<string> infos = print_file_list("./" + string(user));
            for (int i = 0; i < infos.size(); i++)
            {
                cout << "write6" << endl;
                sendMessage((char*)infos.at(i).c_str(), 1, MENSAGEM_ITEM_LISTA_DE_ARQUIVOS , 1, user, sockfd);
            }
            sendMessage("", 1, ACK , 1, user, sockfd);
        }
        if (pkt.type == MENSAGEM_DELETAR_NO_SERVIDOR){
            string toRemoveFilePath;

            toRemoveFilePath = string(pkt._payload);
            toRemoveFilePath = toRemoveFilePath.substr(toRemoveFilePath.find_last_of("/") + 1);
            string file_path = "./";
            file_path = file_path + pkt.user + "/" + toRemoveFilePath;

            int result = delete_file(file_path);

            if(result == -1){
				cout << "could not delete file" << endl;
                cout << "file path:" << endl;
			    cout << file_path << endl;
			}

            vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

            cout << "toRemoveFilePath: " << toRemoveFilePath << endl;

            for(int i = 0; i < sync_dir_sockets.size(); i++){
                cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << endl;
                cout << "write8" << endl;
                sendMessage((char *)toRemoveFilePath.c_str(), 1, MENSAGEM_DELETAR_NOS_CLIENTES, 1, user, sync_dir_sockets[i]); // pedido de delete enviado para o cliente
            }
        }
        if (pkt.type == MENSAGEM_DOWNLOAD_NO_SERVIDOR){
            string directory = "./";
            directory = directory + pkt.user + "/" + string(pkt._payload);
            send_file_to_client(sockfd,user,directory);
        }
    }

    return 0;
}

void handle_ctrlc(int s){
    close_connections();
    END = true;
}

void close_connections(){
    for(int i = 0; i < connections.size(); i++){
        close(connections[i]);
    }
    connections = {};
}

int send_file_to_client(int sock, char username[], std::string file_path)
{
	char buffer[BUFFER_SIZE];
    PACKET pktreceived;
	ifstream file;
    cout<< "file_path: " << file_path << endl;
	file.open(file_path, ios_base::binary);
	if (!file.is_open())
	{
		cout << " falha ao abrir"
			 << "\n"
			 << endl;
        cout << "write10" << endl;
        sendMessage("", 1, MENSAGEM_FALHA_ENVIO, 1, username, sock);
		return 0;
	}
	else
	{
		file.seekg(0, file.end);
		uint32_t file_size = file.tellg();
		cout << file_size << "\n";
		file.clear();
		file.seekg(0);

        uint32_t max_fragments = 0;
        max_fragments = file_size/BUFFER_SIZE;

		if(file_size % BUFFER_SIZE != 0){
			max_fragments++;
		}

        cout << "max_fragments: " << max_fragments << endl;

        cout << "write11" << endl;
		sendMessage((char *)file_path.c_str(), file_size, MENSAGEM_ENVIO_NOME_ARQUIVO, max_fragments, username, sock);
        int i;
        int counter = 0;
		for (i = 0; i < file_size; i += ((sizeof(buffer)))) // to read file
		{
            //cout << "i: " << i << endl;
            //cout << "counter: " << counter << endl;
			memset(buffer, 0, BUFFER_SIZE);
			file.read(buffer, sizeof(buffer));
            for(int j =0;j<BUFFER_SIZE; j++){
                //printf("%x ", (unsigned char)buffer[i]);
            }
            
            cout << "write12" << endl;
			sendMessage(buffer, 1, MENSAGEM_ENVIO_PARTE_ARQUIVO, max_fragments, username, sock);
            counter++;
            cout << "counter: " << counter << endl;
            /*if(counter % 300 == 299){
                cout << "read30" << endl;
                readSocket(&pktreceived,sock);
            }*/
		}
        cout << "max_fragments: " << max_fragments << endl;
        cout << "write13" << endl;
        sendMessage(buffer, 1, MENSAGEM_ARQUIVO_LIDO, max_fragments, username, sock);
		file.close();
		cout << "arquivo lido"
			 << "\n"
			 << endl;
		return 1;
	}
}