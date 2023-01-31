#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

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

    create_update_handler(); // cria um thread para lidar com atualizações enviadas pelo servidor e cuida dessas atualizações via handle_updates()
    
    while(true){
        send_request();

        if(exit){
            break;
        }
    }
    return 0;
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

handle_updates(){
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

bool check_for_server_update(){
    //lê do socket e retorna true se recebe algo
}

handle_server_update(){
    //ao receber atualiza o diretório local com o arquivo recebido
}

bool check_for_client_update(){
    //verifica mudanças no diretório
}

handle_client_update(){
    //envia as mudanças para o servidor
}

handle_first_sync(){
    //cria o diretório sync_dir se necessário
    //recebe vários arquivos do servidor e espera pela mensagem de fim de first_sync
}

send_login_request(string username){
    //TO DO
}

send_request(){
    string input = read_input();

    List<string> args = get_args(input);

    send_request(args);
}

send_request(List<string> args){
    string type = args[0];
    //sempre esperar pela mensagem de confirmação do servidor
    switch(type){
        case "upload_file":
            //send upload request with file
            break;
        case "download_file":
            //send download request
            break;
        case "delete_file":
            //send delete request
            break;
        case "list_files_from_server":
            //send list_files request
            break;
        case "list_files_from_client":
            //show client files list
            break;
        case "get_sync_dir":
            if(!sync_online){
                sync();
            }
            else{
                //mensagem de erro
            }
            break;
        case "exit_connection":
            //send exit connection request
            //exit program
            break;
        case "error":
            //exibir mensagem de erro
            //pedir para o usuário digitar um comando válido
            break;
        default:
            //exibir mensagem de erro
            //pedir para o usuário digitar um comando válido
            break;
    }
}

//move to socket manager
bool init_connection(){
    //TO DO
}

get_operation(){
    //ler operação da entrada
}