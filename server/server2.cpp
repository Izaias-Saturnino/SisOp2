#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <exception>

#include "./server.hpp"

using namespace std;

#define PORT 4000

int sockfd,newSockfd;
struct sockaddr_in cli_addr;
pthread_t clientThread;
socklen_t clilen;

//client socket handler variables
thread_local char user[256];
thread_local int sockfd, newSockfd;
thread_local PACKET pkt;
thread_local string username;
thread_local string file_relative_path;
thread_local string server_path;
thread_local string user_folder_path;

LoginManager *loginManager = new LoginManager();

//move to socket manager
//talvez dividir em open_socket() e bind_socket()
bool open_server(int argc, char *argv[]){
    bool server_is_open = true;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    verificaRecebimentoIP(argc, argv);

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        imprimeServerError();
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){  ///Verifica IP valido
        std::cerr << "ERROR opening socket" << std::endl;
        server_is_open = false;
    }
        
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(PORT);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cerr << "ERROR binding socket" << std::endl;
        server_is_open = false;
    }

    return server_is_open;
}

//move to socket manager
bool listen_for_clients(){
    /*listen to clients*/
    listen(sockfd, 5);
    clilen = sizeof(struct sockaddr_in);
}

//move to socket manager
bool accept_connection(){
    return (newSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) != -1;
}

bool create_connection_handler(){
    // cria thread que lida com a conexão separadamente chamando o handle_connection()
    bool connection_handler_created = true;
    try{
        pthread_create(&clientThread, NULL, handle_connection, &newSockfd);
    }
    catch(exception &e){
        connection_handler_created = false;
    }
    return connection_handler_created;
}

//move to socket manager
bool exit_connection(){
    //tenta fechar a conexão com o cliente
    //retorna true se consegue
    //retorna false se não consegue
}

void exit(){
    //fechar as conexões
    //fechar o servidor
    //terminar programa
    exit(0);
}

int main(int argc, char *argv[]){
    verificaRecebimentoIP(argc, argv);

    bool server_is_up = open_server(argc, argv);
    if(!server_is_up){
        std::cerr << "Could not open server" << std::endl;
        exit(0);
    }

    while(true){
        listen_for_clients();
        bool connection_accepted = accept_connection();
        if(connection_accepted){
            bool connection_handler_created = create_connection_handler();
            if(!connection_handler_created){
                std::cerr << "Could not create thread to handle connection" << std::endl;
                //código complicado para lidar com esse erro que não está no escopo desse trabalho deveria ser introduzido aqui
            }
        }
        else{
            std::cerr << "ERROR on accept" << std::endl;
        }
    }
    return 0;
}

//move to socket manager
bool read_request(){
    //se não tiver nada, só retornar false
    //se tiver algo, ler o socket até ter todos os dados necessários para tratar a comunicação
    //quando terminar de obter os dados, gravar os dados em um pacote e retornar true
    //se tiver erro nos dados, a ponto de não conseguir tratar, mandar uma mensagem de erro ao cliente (se possível) e retornar false
}

string get_request_type(){
    //lê a request e retorna o tipo dela
}

void send_message(vector<string> args){
    string message_type = args[0];

    if(message_type == "file"){
        //send download answer
    }
    else if(message_type == "list_files_from_server"){
        //send delete answer
    }
    else if(message_type == "could_not_read_request"){
        //send mensagem de erro de comando não entendido
    }
    else if(message_type == "first_sync_end"){
        //mandar mensagem de fim de sincronização inicial
    }
    else if(message_type == "login"){
        //mandar mensagem de login (no payload da mensagem diz se deu erro ou não)
    }
    else if(message_type == "undefined_error"){
        //mandar mensagem de comando não entendido
    }
    else{
        std::cerr << "Message answer type not defined" << std::endl;
    }
}

// define todas as variáveis relevantes após a leitura com sucesso da requisição
// essas variáveis são: username, tipo de request, remetente, request_from_new_device, request_from new user, etc...

bool auth(){
    //realiza o login
    bool auth_successful = loginManager->login(newSockfd,user);

    return auth_successful;
}

void get_login_data(){
    //pega os dados de login do payload da request de login
    strcpy(user,pkt.user);
}

bool handle_login(){
    read_request();
    string request_type = get_request_type();

    bool login_successful;

    if(request_type != "login"){
        login_successful = false;
    }
    else{
        get_login_data();

        login_successful = auth();
    }

    return login_successful;
}

//move to dir manager
bool create_user_folder(){
    //cria um diretório para o usuário
}

vector<string> get_removed_files_list(){
    //verifica os arquivos que estão no diretório do cliente, mas não no servidor e coloca o  nome deles na lista
}

vector<string> get_new_files_list(){
    //verifica os arquivos que estão no servidor, mas faltam para o cliente e coloca o nome deles na lista
}

void send_file_to_clients(string file_relative_path){
    //cria uma mensagem a partir do arquivo e envia ao cliente
}

void send_files_to_clients(vector<string> files_to_send){

}

void send_all_files_to_client(){
    //pega a lista de arquivos do servidor
    //envia todos os arquivos via send_files_to_clients()
}

void remove_files_on_clients(vector<string> files_to_remove){
    //envia uma mensagem do tipo remove para o cliente com uma lista de arquivos para remover
}

void handle_first_sync(){
    bool request_from_new_user = true || false;
    bool request_from_new_device = request_from_new_user || true || false;

    if(request_from_new_user){
        create_user_folder();
    }

    else if(request_from_new_device){
        send_all_files_to_client();
    }

    bool server_client_are_consistent = true || false;

    if(!server_client_are_consistent){
        vector<string> removed_files_list = get_removed_files_list();
        vector<string> new_files_list = get_new_files_list();

        remove_files_on_clients(removed_files_list);
        send_files_to_clients(new_files_list);
    }

    vector<string> first_sync_end = {"first_sync_end"};
    send_message(first_sync_end);
}

//move to dir manager
void delete_file(string file_relative_path){
    //remove o aquivo do servidor
}

//move to dir manager
void create_file_from_request(){
    //cria um arquivo a partir do payload request
}

//move to dir manager
string list_files_in_sync_dir(){
    //cria uma lista com todos os arquivos atuais do servidor em formato de texto
}

void handle_request(){
    read_request();
    string request_type = get_request_type();
    //aqui fica todo o código que cuida das requisições de comunicação feitas pelo cliente ao servidor, menos as requisições de login
    if(request_type == "download_file"){
        vector<string> args = {"file", file_relative_path};
        send_message(args);
    }
    else if(request_type == "delete_file"){
        delete_file(file_relative_path);

        vector file = {file_relative_path};
        remove_files_on_clients(file);
    }
    else if(request_type == "upload_file"){
        create_file_from_request();

        send_file_to_clients(file_relative_path);
    }
    else if(request_type == "list_files_from_server"){
        string list = list_files_in_sync_dir();

        vector<string> args = {"list_files_from_server", list};
        send_message(args);
    }
    else if(request_type == "could_not_read_request"){
        vector<string> args = {"could_not_read_request"};
        send_message(args);
    }
    else if(request_type == "logout"){
        char resposta[40];
        loginManager->Logout(user,sockfd,resposta);
    }
    else{
        vector<string> args = {"undefined_error"};
        send_message(args);
    }
}

void* handle_connection(void *arg){
    sockfd= *(int *) arg;
    bool login_successful = handle_login();

    vector<string> args;
    args.push_back("login");

    if(login_successful){
        args.push_back("OK");
        send_message(args);
    }
    else{
        args.push_back("ERROR");
        send_message(args);
        close(newSockfd);
        return 0;
    }

    handle_first_sync();

    while(true){
        handle_request();
    }
}