#include "./server.hpp"

#define PORT 4000

LoginManager *loginManager = new LoginManager();
char user[BUFFER_SIZE];
int newSockfd,connectionSocket;
bool END = false;
vector<int> connections;

typedef struct sock_and_user{
    int sock;
    char user[BUFFER_SIZE];
}SOCK_USER;

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

    if ((connectionSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){ /// Verifica IP valido
        printf("ERROR opening socket\n");
        return 0;
    }

    connections.push_back(connectionSocket);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    cout << "starting server" << endl;
    while(bind(connectionSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        //cout << "ERROR on binding" << endl;
        sleep(1);
    }
    cout << "server started" << endl << endl;
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
            strcpy(user, pkt._payload);
            if(pkt.type == MENSAGEM_LOGIN){
                usuarioValido = loginManager->login(newSockfd, user);

                if (usuarioValido)
                {
                    string path = "./" + string(user);
                    cout << path << "\n";
                    cout << "user " + string(user) + " logged in" << endl;
                    if (!filesystem::is_directory(path))
                    {
                        cout << path << "\n";
                        create_folder(path);
                    }
                    cout << "write1" << endl;
                    sendMessage("", MENSAGEM_USUARIO_VALIDO, newSockfd); // Mensagem de usuario Valido

                    SOCK_USER sock_and_user;
                    sock_and_user.sock = newSockfd;
                    strcpy(sock_and_user.user, user);

                    while(pthread_create(&clientThread, NULL, ThreadClient, &sock_and_user) != 0){ // CUIDADO: newSocket e não socket
                        cout << "ERROR creating client thread. retrying..." << endl;
                    }
                }
                else
                {
                    cout << "write2" << endl;
                    sendMessage((char*)"Excedido numero de sessoes", MENSAGEM_USUARIO_INVALIDO, newSockfd); // Mensagem de usuario invalido
                }
            }
            else if(pkt.type == GET_SYNC_DIR){
                //baixar todos os arquivos do syncdir do servidor
                vector<string> file_paths = get_file_list("./" + string(user));

                for(int i = 0; i < file_paths.size(); i++){
                    send_file_to_client(newSockfd, file_paths[i]);
                }

                cout << "write3" << endl;
                sendMessage("", FIRST_SYNC_END, newSockfd);
        
                //salvar o socket que pediu atualizações de sync dir
                loginManager->activate_sync_dir(user, newSockfd);

                cout << string(user) << " actived sync dir" << endl;
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
    SOCK_USER sock_user = *(SOCK_USER *)arg;
    int sockfd = sock_user.sock;
    char user[BUFFER_SIZE];
    strcpy(user, sock_user.user);

    PACKET pkt;
    char resposta[40];
    ofstream file_server;
    int received_fragments=0;
    vector<vector<char>> fragments = {};
    string directory;

    bool file_received_from_sync = false;

    uint32_t remainder_file_size = 0;

    while (true)
    {
        memset(pkt._payload, 0, BUFFER_SIZE);
        cout << "read1" << endl;
        readSocket(&pkt, sockfd);
        cout << "pkt.type: " << pkt.type << ". ";
 
        if (pkt.type == MENSAGEM_LOGOUT)
        {
            cout << string(user) << " logout" << endl;
            loginManager->Logout(user, sockfd, resposta);
            break;
        }
        if (pkt.type == MENSAGEM_ENVIO_SYNC){
            file_received_from_sync = true;
        }
        if (pkt.type == MENSAGEM_ENVIO_NOME_ARQUIVO)
        {
            string file_name = getFileName(string(pkt._payload));
            cout << "receiving file" << endl;
            cout << "file name: " << file_name << endl;
            string directory = "./";
            directory = directory + user + "/" + file_name;
            cout << directory << "\n" << endl;

            receiveFile(sockfd, directory, &pkt);

            cout << "file reasembled" << endl;

            vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

            cout << "file location: " << directory << endl;

            cout << "sync_dir_sockets.size(): " << sync_dir_sockets.size() << endl;

            cout << "propagating file" << endl;
            for(int i = 0; i < sync_dir_sockets.size(); i++){
                if(file_received_from_sync){
                    if(sync_dir_sockets[i] == loginManager->get_sender_sync_sock(sockfd)){
                        cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << " não recebe o arquivo." << endl;
                        continue;
                    }
                }
                cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << endl;
                send_file_to_client(sync_dir_sockets[i], directory);
            }
            cout << "file propagated" << endl;
            file_received_from_sync = false;
        }
        if (pkt.type == MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR)
        {
            cout << "sending " << string(user) << " file list" << endl;
            vector<string> infos = print_file_list("./" + string(user));
            for (int i = 0; i < infos.size(); i++)
            {
                cout << "write6" << endl;
                sendMessage((char*)infos.at(i).c_str(), MENSAGEM_ITEM_LISTA_DE_ARQUIVOS, sockfd);
            }
            cout << "ack MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR" << endl;
            sendMessage("", ACK, sockfd);
        }
        if (pkt.type == MENSAGEM_DELETAR_NO_SERVIDOR){
            string file_name = getFileName(string(pkt._payload));
            cout << "removing file" << endl;
            cout << "file name: " << file_name << endl << endl;
            string file_path = "./";
            file_path = file_path + user + "/" + file_name;

            int result = delete_file(file_path);

            if(result == -1){
				cout << "could not delete file" << endl;
                cout << "file path:" << endl;
			    cout << file_path << endl;
			}

            vector<int> sync_dir_sockets = loginManager->get_active_sync_dir(user);

            cout << "file_name: " << file_name << endl;

            for(int i = 0; i < sync_dir_sockets.size(); i++){
                cout << "sync_dir_sockets[" << i << "]: " << sync_dir_sockets[i] << endl;
                cout << "write8" << endl;
                sendMessage((char *)file_name.c_str(), MENSAGEM_DELETAR_NOS_CLIENTES, sync_dir_sockets[i]); // pedido de delete enviado para o cliente
            }
        }
        if (pkt.type == MENSAGEM_DOWNLOAD_FROM_SERVER){
            string directory = "./";
            directory = directory + user + "/" + string(pkt._payload);
            send_file_to_client(sockfd, directory);
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

void send_file_to_client(int sock, string file_path)
{
	sendFile(sock, file_path);
}