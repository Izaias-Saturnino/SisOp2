#include "./server.hpp"

#define PORT 4000

LoginManager *loginManager = new LoginManager();

int main(int argc, char *argv[])
{
    int sockfd, newSockfd;
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;
    pthread_t clientThread;
    socklen_t clilen;
    char user[256];
    bool usuarioValido;
    PACKET pkt;

    verificaRecebimentoIP(argc, argv);

    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        imprimeServerError();
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) /// Verifica IP valido
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(0);
    }
    else
    {
        while (true)
        {

            /*listen to clients*/
            listen(sockfd, 5);
            clilen = sizeof(struct sockaddr_in);

            if ((newSockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1)
                printf("ERROR on accept");

            else
            {
                /*inicio login*/
                readSocket(&pkt, newSockfd);
                strcpy(user, pkt.user);
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
                    sendMessage("OK", 1, MENSAGEM_USUARIO_VALIDO, 1, user, newSockfd);                   // Mensagem de usuario Valido
                    pthread_create(&clientThread, NULL, ThreadClient, &newSockfd); // CUIDADO: newSocket e não socket
                }
                else
                {
                    sendMessage("Excedido numero de sessoes", 1, MENSAGEM_USUARIO_INVALIDO, 1, user, newSockfd); // Mensagem de usuario invalido
                }
            }
        }
    }

    close(newSockfd);
    close(sockfd);

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
    string directory = "/sync_dir/";
    ofstream file_server;
    int size=0;
    int received_fragments=0;
    vector<vector<char>> fragments(10);
    while (true)
    {
        memset(pkt._payload, 0, 256);
        readSocket(&pkt, sockfd);
        strcpy(user, pkt.user);

        if (pkt.type == MENSAGEM_LOGOUT)
        {
            loginManager->Logout(user, sockfd, resposta);
            sendMessage(resposta, 1, MENSAGEM_RESPOSTA_LOGOUT, 1, user, sockfd); // resposta logout
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
            fragments.resize(size+1);
            received_fragments =0;
        }
        if(pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO || pkt.type == MENSAGEM_ARQUIVO_LIDO)
        {
            char buffer [256];
            vector<char> bufferconvert(256);

            memset(buffer, 0, 256);

            memcpy(buffer,pkt._payload, 256);

            printf("%d",received_fragments);

            for(int i = 0; i < bufferconvert.size(); i++){
                bufferconvert[i] = buffer[i];
            }
            
            if (pkt.type == MENSAGEM_ENVIO_PARTE_ARQUIVO)
            {
                received_fragments++;
                fragments.at(pkt.seqn)=bufferconvert;
            }
            if(received_fragments == size+1)
            {
                for (int i =0 ;i<fragments.size();i++){
                    for(int j=0;j<fragments.at(i).size();j++){
                        char* frag = &(fragments.at(i).at(j));
                        printf("%x ",(unsigned char)fragments.at(i).at(j));
                        file_server.write(frag, sizeof(char));
                    }
                }
                file_server.close();
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
    }

    return 0;
}

void receive_file_client(int sock, char username[])
{
    string fileName;
    PACKET receivedFilePathPacket;
    string receivedFilePath;

    receivedFilePath = string(receivedFilePathPacket._payload);
    receivedFilePath.substr(receivedFilePath.find_last_of("/") + 1);
}