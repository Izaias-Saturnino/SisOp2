#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>

using namespace std;

thread_local string username;
thread_local string file_relative_path;
thread_local string server_path;
thread_local string user_folder_path;

//move to socket manager
bool open_server(){
    //tenta abrir o servidor
    //retorna true se consegue
    //retorna false se não consegue
}

//move to socket manager
bool listen_for_clients(){
    //tenta fazer o servidor começar a ouvir conexões de clientes
    //retorna true se consegue
    //retorna false se não consegue
}

//move to socket manager
bool accept_connection(){
    //tenta aceitar a conexão do cliente
    //retorna true se consegue
    //retorna false se não consegue
}

bool create_connection_handler(){
    // cria thread que lida com a conexão separadamente chamando o handle_connection()
}

//move to socket manager
bool exit_connection(){
    //tenta fechar a conexão com o cliente
    //retorna true se consegue
    //retorna false se não consegue
}

int main(){
    bool server_is_up = open_server();
    if(!server_is_up){
        std::cerr << "Could not open server" << std::endl;
        return 1;
    }

    bool server_is_listening = listen_for_clients();
    if(!server_is_listening){
        std::cerr << "Could not listen to connections" << std::endl;
        return 1;
    }

    while(true){
        bool connection_accepted = accept_connection();
        if(connection_accepted){
            bool connection_handler_created = create_connection_handler();
            if(!connection_handler_created){
                std::cerr << "Could create thread to handle connection" << std::endl;
                //código complicado para lidar com esse erro que não está no escopo desse trabalho deveria ser introduzido aqui
            }
        }
    }
    return 0;
}

//move to socket manager
bool read_from_socket(){
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
    else if(message_type == "login_error"){
        //mandar mensagem de erro de login
    }
    else if(message_type == "undefined_error"){
        //mandar mensagem de comando não entendido
    }
    else{
        std::cerr << "Message answer type not defined" << std::endl;
    }
}


// define todas as variaveis relevantes apos a leitura com sucesso da requisicao
// essas variaveis são: username, tipo de request, remetente, etc...
string read_request(){
    string type;

    bool read_successful = read_from_socket();

    if(!read_successful){
        type = "could_not_read_request";
    }else{
        type = get_request_type();
    }

    return type;
}

bool auth(string login_data){
    //realiza o login
    user_folder_path = server_path + "/" + username;
    //por enquanto só aceita
    return true;
}

string get_login_data(){
    //pega os dados de login do payload da request de login
}

bool handle_login(){
    string request_type = read_request();

    vector<string> args;

    bool login_successful;

    if(request_type != "login"){
        args.push_back("login_error");
        send_message(args);

        login_successful = false;
    }
    else{
        string login_data = get_login_data();

        login_successful = auth(login_data);
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
    bool new_device = request_from_new_user || true || false;

    if(request_from_new_user){
        create_user_folder();
    }

    else if(new_device){
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
    string request_type = read_request();
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
    else{
        vector<string> args = {"undefined_error"};
        send_message(args);
    }
}

void handle_connection(){
    bool login_successful = handle_login();
    if(!login_successful){
        return;
    }

    bool device_first_sync = true || false;
    if(device_first_sync){
        handle_first_sync();
    }

    while(true){
        handle_request();
    }
}