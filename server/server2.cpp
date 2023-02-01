#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>
#include <exception>
#include <cstdio>
#include <sys/stat.h>
#include <filesystem>
namespace fs = std::filesystem;

#include "./server.hpp"

using namespace std;

#define PORT 4000

int newSockfd;
struct sockaddr_in cli_addr;
pthread_t clientThread;
socklen_t clilen;

//client socket handler variables
thread_local char user[256];
thread_local int sockfd;
thread_local PACKET pkt;
thread_local string file_relative_path; //relative to user folder on server
thread_local string server_path = "sync_dirs/"; //server sync_dir folder absolute path
thread_local string user_folder_name;
thread_local bool request_from_new_device;
thread_local bool end_connection;

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
    listen(sockfd, 5);
    clilen = sizeof(struct sockaddr_in);
}

//move to socket manager
bool accept_connection(){
    return (newSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) != -1;
}

// cria thread que lida com a conexão separadamente chamando o handle_connection()
bool create_connection_handler(){
    bool connection_handler_created = true;
    try{
        pthread_create(&clientThread, NULL, handle_connection, &newSockfd);
    }
    catch(exception &e){
        connection_handler_created = false;
    }
    return connection_handler_created;
}

//fecha o servidor adequadamente
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
void read_request(){
    readSocket(&pkt,newSockfd);
}

//lê a request e retorna o tipo dela
string get_request_type(){
    string type;
    switch(pkt.type){
        case 1:
            type = "some type";
            break;
        default:
            type = "error";
            break;
    }
    return type;
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

//realiza o login
bool auth(){
    bool auth_successful = loginManager->login(newSockfd,user);
    return auth_successful;
}

//pegar os dados de login do payload da request de login
void get_login_data(){
    //pegar os dados de login do payload da request de login
    //essas variáveis são: user, request_from_new_device, request_from new user, etc.
    //poderia adicionar password para mais segurança
    strcpy(user,pkt.user);
    user_folder_name = user;
}

//lida com a request de login
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



vector<string> get_client_file_list(){
    //mandar mensagem ao cliente pedindo uma lista de arquivos
}

vector<string> get_server_file_list(){
    string user_folder_path = server_path+user;
    return get_file_list(user_folder_path);
}




void send_file_to_clients(string file_relative_path){
    //cria uma mensagem a partir do arquivo e envia ao cliente
}

void send_files_to_clients(vector<string> files_to_send){
    for(int i = 0; i < files_to_send.size(); i++){
        send_file_to_clients(files_to_send[i]);
    }
}

void send_all_files_to_client(){
    vector<string> server_file_list = get_server_file_list();
    send_files_to_clients(server_file_list);
}

void remove_files_on_clients(vector<string> files_to_remove){
    //envia uma mensagem do tipo remove para o cliente com uma lista de arquivos para remover
}

bool check_if_user_exists(string user_folder_path){
    struct stat sb;
    return stat(user_folder_path.c_str(), &sb) == 0;
}

void handle_first_sync(){
    string user_folder_path = server_path.c_str()+user_folder_name;

    bool request_from_new_user = !check_if_user_exists(user_folder_path);
    request_from_new_device = request_from_new_user || request_from_new_device;

    if(request_from_new_user){
        bool created_user_folder = create_folder(user_folder_path);
        if(!created_user_folder){
            cerr << "Could not create user folder" << endl;
        }
    }

    else if(request_from_new_device){
        send_all_files_to_client();
    }

    bool server_client_are_consistent = true || false;

    if(!server_client_are_consistent){

        vector<string> client_file_list = get_client_file_list();
        vector<string> server_file_list = get_server_file_list();
        vector<string> removed_files_list = get_removed_files_list(server_file_list, client_file_list);
        vector<string> new_files_list = get_new_files_list(server_file_list, client_file_list);

        remove_files_on_clients(removed_files_list);
        send_files_to_clients(new_files_list);
    }
}



void handle_request(){
    read_request();
    string request_type = get_request_type();
    //aqui fica todo o código que cuida das requisições de comunicação feitas pelo cliente ao servidor, menos as requisições de login
    if(request_type == "download_file"){
        //string file_relative_path_on_client = pkt.file_relative_path;
        //string file_relative_path = server_path + user_folder_name + file_relative_path_on_client;
        vector<string> args = {"file", file_relative_path};
        send_message(args);
    }
    else if(request_type == "delete_file"){
        int result = delete_file(file_relative_path);

        if(result != 0){
            std::cerr << "ERROR in deleting file" << std::endl;
        }

        vector file = {file_relative_path};
        remove_files_on_clients(file);
    }
    else if(request_type == "upload_file"){
        create_file_from_request();

        send_file_to_clients(file_relative_path);
    }
    else if(request_type == "list_files_from_server"){
        vector<string> list = get_server_file_list();

        vector<string> comm = {"list_files_from_server"};

        vector<string> args;

        args.reserve(list.size() + comm.size());
        args.insert(args.end(), comm.begin(), comm.end());
        args.insert(args.end(), list.begin(), list.end());

        send_message(args);
    }
    else if(request_type == "could_not_read_request"){
        vector<string> args = {"could_not_read_request"};
        send_message(args);
    }
    else if(request_type == "logout"){
        char resposta[40];
        loginManager->Logout(user,sockfd,resposta);
        end_connection = true;
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
        close(sockfd);
        return 0;
    }

    handle_first_sync();

    while(true){
        handle_request();
        if(end_connection){
            break;
        }
    }
}