#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main(){
    bool connection_successful = init_connection();
    if(!connection_successful){
        std::cerr << "Connection error" << std::endl;
        return 1; // acho que seria melhor fazer um retry
    }
    bool login_successful = attempt_login();
    if(!login_successful){
        std::cerr << "Login error" << std::endl;
        return 1;
    }
    sync(); // cria um thread para lidar com atualizações enviadas pelo servidor e cuida dessas atualizações via handle_updates()
    while(true){
        send_request();

        if(exit){
            break;
        }
    }
    return 0;
}

handle_updates(){
    //esperar pelo primeiro pacote do servidor e verificar se ele está pedindo um handle_first_sync()
    bool first_sync = ...;
    if(first_sync){
        handle_first_sync();
    }
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

bool init_connection(){
    //TO DO
}

attempt_login(){
    //TO DO
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
    //receber vários arquivos do servidor e esperar pela mensagem de fim de first_sync
}

get_operation(){
    //ler operação da entrada
}

send_request(){
    string input = read_input();

    string type = get_operation(input);

    //sempre esperar pela mensagem de confirmação do servidor
    switch(type){
        case "download_file":
            //send download request
            break;
        case "delete_file":
            //send delete request
            break;
        case "upload_file":
            //send upload request with file
            break;
        case "list_files_from_server":
            //send list_files request
            break;
        case "exit_connection":
            //send exit connection request
            //exit program
            break;
        default:
            //exibir mensagem de erro
            break;
    }
}
