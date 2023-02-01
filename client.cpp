#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace std;

bool sync_online = false;
bool exit_program = false;

//move to socket manager
bool init_connection(){
    //TO DO
}

string read_input(){
    //read input and put it in a string
}

void send_login_request(string username){
    //TO DO
}

string read_request(){
    //string type = ...;

    //inicia as variaveis que são recebidas da mensagem
}

bool attempt_login(){
    bool successful = true;

    //pegar dados de login

    string username = read_input();
    //string password = ...;

    //envia os dados em uma mensagem de login
    send_login_request(username);

    string request_type = read_request();
    if(request_type != "success"){
        successful = false;
    }

    return successful;
}

void create_update_handler(){

}

vector<string> get_args(string input){
    //processar entrada e dividir em argumentos para o vetor
}

void send_request(vector<string> args){
    string type = args[0];
    //sempre esperar pela mensagem de confirmação do servidor
    if(type == "upload_file"){
        //send upload request with file
    }
    else if(type == "download_file"){
        //send download request
    }
    else if(type == "delete_file"){
        //send delete request
    }
    else if(type == "list_files_from_server"){
        //send list_files request
    }
    else if(type == "list_files_from_client"){
        //show client files list
    }
    else if(type == "get_sync_dir"){
        if(!sync_online){
            sync_online = true;
            create_update_handler(); // cria um thread para lidar com atualizações enviadas pelo servidor e cuida dessas atualizações via handle_updates()
        }
        else{
            //mensagem de erro
        }
    }
    else if(type == "exit_connection"){
        //exit connection
        exit_program = true;
    }
    else if(type == "error"){
        //exibir mensagem de erro
        //pedir para o usuário digitar um comando válido
    }
    else{
        //exibir mensagem de erro
        //pedir para o usuário digitar um comando válido
    }
}

void send_request(){
    string input = read_input();

    vector<string> args = get_args(input);

    send_request(args);
}

void handle_first_sync(){
    //cria o diretório sync_dir se necessário
    //recebe vários arquivos do servidor e espera pela mensagem de fim de first_sync
}

bool check_for_server_update(){
    //lê do socket e retorna true se recebe algo
}

void handle_server_update(){
    //ao receber a atualizacao atualiza o diretório local com o arquivo recebido
}

bool check_for_client_update(){
    //verifica mudanças no diretório
}

void handle_client_update(){
    //envia as mudanças para o servidor
}

void handle_updates(){
    //esperar pelo primeiro pacote do servidor e verificar se ele está pedindo um handle_first_sync()

    //bool first_sync = ...;
    //if(first_sync){
        handle_first_sync();
    //}

    while(true){
        bool server_update = check_for_server_update();
        if(server_update){
            handle_server_update();
        }
        bool client_update = check_for_client_update();
        if(client_update){
            handle_client_update();
        }
    }
}

int main(){
    bool connection_successful = init_connection();
    if(!connection_successful){
        std::cerr << "Connection error" << std::endl;
        return 1;
    }

    bool login_successful = attempt_login();
    while(!login_successful){
        return 1;
    }

    while(true){
        send_request();

        if(exit_program){
            break;
        }
    }
    return 0;
}