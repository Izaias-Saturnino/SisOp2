#include "./server.hpp"

#define PORT 4000

LoginManager *loginManager = new LoginManager();
char user[BUFFER_SIZE];
int newSockfd,connectionSocket;
bool END = false;
vector<int> client_connections;

bool main_server;
SERVER_COPY this_server;

vector<SERVER_COPY> servers;

typedef struct sock_and_user{
    int sock;
    char user[BUFFER_SIZE];
}SOCK_USER;

int main(int argc, char *argv[])
{
    this_server.ip = argv[1];
    servers.push_back(this_server);
    if(argc > 2){
        char* other_server_ip = argv[2];
        int other_server_socket = connect_to_server(other_server_ip);
        if(other_server_socket == -1){
            cout << "ERROR opening server socket\n";
        }
        send_list_of_servers(other_server_socket);
        receive_list_of_servers(other_server_socket);
        close(other_server_socket);

        broadcast_new_server(this_server);

        int main_server_socket = connect_to_main_server();

        if(!main_server){
            this_server.id = request_id(main_server_socket);
            check_main_server_up(main_server_socket);//create thread for this update
        }
        else{
            this_server.id = rand();
        }
    }

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

    server = gethostbyname(this_server.ip);

    if (server == NULL)
    {
        imprimeServerError();
    }

    if ((connectionSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){ /// Verifica IP valido
        cout << "ERROR opening socket\n";
        return 0;
    }

    client_connections.push_back(connectionSocket);

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
            client_connections.push_back(newSockfd);

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
    
    cout << endl << "closing client_connections" << endl;
    close_client_connections();

    return 0;
}

void handle_client_packet(PACKET pkt, int socket){
    //bring the handling from the switch here
}

char* server_copy_to_buffer(SERVER_COPY server_copy){
    char server_copy_buffer[BUFFER_SIZE];

	char *q = (char*)server_copy_buffer;
    *q = server_copy.id;
	q += sizeof(server_copy.id);

    int i = sizeof(server_copy.id);
    while(i < BUFFER_SIZE){
        *q = server_copy.ip[i];
        q++;
        i++;
    }

    return server_copy_buffer;
}

SERVER_COPY server_copy_from_buffer(char* server_copy_buffer){
    SERVER_COPY server_copy;

	char *q = (char*)server_copy_buffer;
    server_copy.id = *q;
	q += sizeof(server_copy.id);

    int i = sizeof(server_copy.id);
    while(i < BUFFER_SIZE){
        server_copy.ip[i] = *q;
        q++;
        i++;
    }

    return server_copy;
}

void broadcast_new_server(SERVER_COPY server_copy){
    vector<int> server_connections;
    for (int i = 0; i < servers.size(); i++)
    {
        if(!host_equals(servers[i].ip, this_server.ip)){
            int socket = connect_to_server(servers[i].ip);
            server_connections.push_back(socket);
        }
    }

    char* server_copy_buffer = server_copy_to_buffer(server_copy);
    for (int i = 0; i < server_connections.size(); i++)
    {
        sendMessage(server_copy_buffer, LIST_SERVER_ITEM, server_connections[i]);
    }

    for (int i = 0; i < server_connections.size(); i++)
    {
        close(server_connections[i]);
    }
}

void send_list_of_servers(int server_socket){
    for(int i = 0; i < servers.size(); i++){
        char* server_copy_buffer = server_copy_to_buffer(servers[i]);
        sendMessage(server_copy_buffer, LIST_SERVER_ITEM, server_socket);
    }
    sendMessage("", ACK, server_socket);
}

void receive_list_of_servers(int server_socket){
    PACKET pkt;
    readSocket(&pkt, server_socket);
    while(pkt.type == LIST_SERVER_ITEM){
        SERVER_COPY server_copy = server_copy_from_buffer(pkt._payload);
        servers.push_back(server_copy);
        readSocket(&pkt, server_socket);
    }
}

bool host_equals(char* ip, char* other_ip){
    struct hostent *server = gethostbyname(ip);
    struct hostent *other_server = gethostbyname(other_ip);
    return server->h_name == other_server->h_name;
}

int connect_to_main_server(){
    int socket = -1;

    SERVER_COPY main_server_copy = this_server;
    for (int i = 0; i < servers.size(); i++)
    {
        if(main_server_copy.id < servers[i].id){
            main_server_copy = servers[i];
        }
    }
    if(main_server_copy.id == this_server.id){
        main_server = true;
    }
    else{
        socket = connect_to_server(main_server_copy.ip);
    }
    return socket;
}

void handle_server_down(int server_socket){
    
}

void check_main_server_up(int server_socket){
    int fail_counter = 0;
    while(true){
        sendMessage("", ACK, server_socket);
        bool message_received = has_received_message(server_socket);
        while(message_received == 0 && fail_counter != 10){
            fail_counter++;
            usleep(20 * 1000);
            message_received = has_received_message(server_socket);
        }
        if(fail_counter == 10){
            handle_server_down(server_socket);
            break;
        }
        fail_counter = 0;
    }
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
    close_client_connections();
    END = true;
}

void close_client_connections(){
    for(int i = 0; i < client_connections.size(); i++){
        close(client_connections[i]);
    }
    client_connections = {};
}

void send_file_to_client(int sock, string file_path)
{
	sendFile(sock, file_path);
}