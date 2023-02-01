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
                    sendMessage("OK", 1, 4, 1, user, newSockfd);                   // Mensagem de usuario Valido
                    pthread_create(&clientThread, NULL, ThreadClient, &newSockfd); // CUIDADO: newSocket e não socket
                }
                else
                {
                    sendMessage("Excedido numero de sessoes", 1, 3, 1, user, newSockfd); // Mensagem de usuario invalido
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

    readSocket(&pkt, sockfd);
    strcpy(user, pkt.user);

    if (pkt.type == 2)
    {
        loginManager->Logout(user, sockfd, resposta);

        sendMessage(resposta, 1, 5, 1, user, sockfd); // resposta logout
    }
    return 0;
}
