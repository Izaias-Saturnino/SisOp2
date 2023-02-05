#include "./server.hpp"

#define PORT 4000

LoginManager *loginManager = new LoginManager();
char user[256];
int newSockfd,conectionSocket;

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

    if ((conectionSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) /// Verifica IP valido
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(conectionSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(0);
    }
    else
    {
        while (true)
        {
            sigaction(SIGINT, &sigIntHandler, NULL);
            /*listen to clients*/
            listen(conectionSocket, 5);
            clilen = sizeof(struct sockaddr_in);

            if ((newSockfd = accept(conectionSocket, (struct sockaddr *)&cli_addr, &clilen)) == -1)
                printf("ERROR on accept");

            else
            {
                /*inicio login*/
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
                        sendMessage("OK", 1, MENSAGEM_USUARIO_VALIDO, 1, user, newSockfd); // Mensagem de usuario Valido
                        pthread_create(&clientThread, NULL, ThreadClient, &newSockfd); // CUIDADO: newSocket e não socket
                    }
                    else
                    {
                        sendMessage("Excedido numero de sessoes", 1, MENSAGEM_USUARIO_INVALIDO, 1, user, newSockfd); // Mensagem de usuario invalido
                    }
                }
                else if(pkt.type == GET_SYNC_DIR){
                    //baixar todos os arquivos do syncdir do servidor
            
                    //salvar o socket que pediu atualizações de sync dir
                    loginManager->activate_sync_dir(user, newSockfd);

                    cout << "active sync dir confirmed" << endl;
                }
            }
        }
    }

    close(newSockfd);
    close(conectionSocket);

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
    char user[256];
    ofstream file_server;
    int size=0;
    int received_fragments=0;
    vector<vector<char>> fragments(10);
    string directory;
    while (true)
    {
        memset(pkt._payload, 0, 256);
        readSocket(&pkt, sockfd);
        strcpy(user, pkt.user);
 
        if (pkt.type == MENSAGEM_LOGOUT)
        {
            loginManager->Logout(user, sockfd, resposta);
            sendMessage(resposta, 1, MENSAGEM_RESPOSTA_LOGOUT, 1, user, sockfd); // resposta logout
            break;
        }
        if (pkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO)
        {

            string receivedFilePath;

            receivedFilePath = string(pkt._payload);
            receivedFilePath = receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
            directory = "./";
            directory = directory + pkt.user + "/" + receivedFilePath;
            file_server.open(directory, ios_base::binary);
            size = pkt.total_size;

            cout << directory << "\n"
                 << endl;

            fragments.clear();
            fragments.resize(size);
            received_fragments =0;
        }
        if(pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || pkt.type == MENSAGEM_ARQUIVO_LIDO)
        {
            char buffer [256];
            vector<char> bufferconvert(256);

            memset(buffer, 0, 256);

            memcpy(buffer,pkt._payload, 256);

            //printf("%d",received_fragments);

            for(int i = 0; i < bufferconvert.size(); i++){
                bufferconvert[i] = buffer[i];
            }
            
            if (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO)
            {
                received_fragments++;
                fragments.at(pkt.seqn)=bufferconvert;
            }
            if(received_fragments %10 ==9){
			    sendMessage("",1,MENSAGEM_DOWNLOAD_NO_SERVIDOR,1,user,sockfd);
		    }
            if(received_fragments == size)
            {
                for (int i =0 ;i<fragments.size();i++){
                    for(int j=0;j<fragments.at(i).size();j++){
                        char* frag = &(fragments.at(i).at(j));
                        //printf("%x ",(unsigned char)fragments.at(i).at(j));
                        file_server.write(frag, sizeof(char));
                    }
                }
                file_server.close();
            }
            if(pkt.type == MENSAGEM_ARQUIVO_LIDO){
                vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

                for(int i = 0; i < sync_dir_sockets.size(); i++){
                    upload_file_server(sync_dir_sockets[i],user,directory);
                }
            }
        }
        if (pkt.type == MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR)
        {
            vector<string> infos = print_file_list("./" + string(user));
            for (int i = 0; i < infos.size(); i++)
            {
                sendMessage((char*)infos.at(i).c_str(), 1, MENSAGEM_ITEM_LISTA_DE_ARQUIVOS , 1, user, sockfd);
                if (i == infos.size() - 1)
                {
                    sendMessage((char*)infos.at(i).c_str(), 1, MENSAGEM_ULTIMO_ITEM_LISTA_ARQUIVOS, 1, user, sockfd);
                }
            }
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

            for(int i = 0; i < sync_dir_sockets.size(); i++){
                sendMessage((char *)toRemoveFilePath.c_str(), 1, MENSAGEM_DELETAR_NOS_CLIENTES, 1, user, sync_dir_sockets[i]); // pedido de delete enviado para o cliente
            }
        }
        if (pkt.type == MENSAGEM_DOWNLOAD_NO_SERVIDOR){
            string directory = "./";
            directory = directory + pkt.user + "/" + string(pkt._payload);
            upload_file_server(sockfd,user,directory);
        }
    }
    return 0;
}

void handle_ctrlc(int s){
	PACKET Pkt;

	cout<<endl<<"Caught signal inside server "<<endl;
	sendMessage("", 1, MENSAGEM_LOGOUT, 1, user, newSockfd); // logout message
	readSocket(&Pkt, newSockfd);
	
	cout << endl << Pkt._payload << endl;

	close(newSockfd);
    close(conectionSocket);

	exit(0);
}
int upload_file_server(int sock, char username[], std::string file_path)
{
	char buffer[256];
    PACKET pktreceived;
	ifstream file;
    cout<<file_path;
	file.open(file_path, ios_base::binary);
	if (!file.is_open())
	{
		cout << " falha ao abrir"
			 << "\n"
			 << endl;
        sendMessage("", 1, MENSAGEM_FALHA_ENVIO, 1, username, sock);
		return 0;
		// mensagem erro
	}
	else
	{
		file.seekg(0, file.end);
		int file_size = file.tellg();
		cout << file_size << "\n";
		file.clear();
		file.seekg(0);

		sendMessage((char *)file_path.c_str(), 1, MENSAGEM_ENVIO_NOME_ARQUIVO, std::ceil(file_size / 256), username, sock);
        sleep(1);
        int i;
        int counter = 0;
		for (i = 0; i < file_size; i += ((sizeof(buffer)))) // to read file
		{
			memset(buffer, 0, 256);
			file.read(buffer, sizeof(buffer));
            for(int i =0;i<256; i++){
                //printf("%x ", (unsigned char)buffer[i]);
            }
            
			sendMessage(buffer, i / 256, MENSAGEM_ENVIO_PARTE_ARQUIVO, 4, username, sock);
            counter++;
            if(counter %10==9){
                readSocket(&pktreceived,sock);
            }
		}
        //sendMessage(buffer, i / 256, MENSAGEM_ARQUIVO_LIDO, 4, username, sock);
        sleep(1);
		file.close();
		cout << " arquivo lido"
			 << "\n"
			 << endl;
		return 1;
	}
}