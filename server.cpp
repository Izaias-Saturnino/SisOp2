#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main(){
    bool connection_is_open = open_connection();
    if(!connection_is_open){
        return 1;
    }

    listen_for_clients();

    while(true){
        bool request_accepted = accept_request();
        if(!request_accepted){
            return 1; // não sei se isso é a melhor maneira de tratar esse erro
        }
        bool read_from_socket = read_request();
        if(request_accepted){
            create_connection_handler(); // thread que lida com a conexão separadamente chamando o handle_connection()
        }
    }
    return 0;
}

bool accept_request(){
    //TO DO
}

bool read_request(){
    //TO DO
}

handle_first_sync(){
    bool request_from_new_user = ...;
    bool new_device = ...;

    if(request_from_new_user){
        create_user_folder();
    }
    else if(new_device){
        send_all_files_to_client();
    }
}

handle_sync(){
    bool server_client_are_consistent = ...;

    if(!server_client_are_consistent){
        bool server_has_new_files = ...;

        if(server_has_new_files){
            send_files_to_client();
        }
    }
}

bool handle_login(){
    //TO DO
}

bool read_from_socket(){
    //se não tiver nada, só retornar false
    //se tiver algo, ler o socket até ter todos os dados necessários para tratar a comunicação
    //quando terminar de obter os dados, gravar os dados em um pacote e retornar true
    //se tiver erro nos dados, a ponto de não conseguir tratar, mandar uma mensagem de erro ao cliente (se possível) e retornar false
}

handle_communication(){
    bool read_successful = read_from_socket();
    if(read_successful){
        //aqui fica todo o código que cuida das requisições de comunicação feitas pelo cliente ao servidor, menos as requisições de login
        string type = ...;
        switch(type){
            case "download_file":
                //TO DO
                break;
            case "delete_file":
                delete_file(file_path);
                break;
            case "upload_file":
                //TO DO
                break;
            case "list_files_from_server":
                //TO DO
                break;
            case "exit_connection":
                //TO DO
                break;
            default:
                //mandar mensagem de erro
                break;
        }
    }
}

handle_connection(){
    bool login_successful = handle_login();
    if(!login_successful){
        // pedir para o usuário tentar novamente
    }

    bool first_sync = ...;
    if(first_sync){
        handle_first_sync();
    }

    while(true){
        handle_communication(); // não se esquecer de mandar mensagens de confirmação para o outro lado sempre que necessário
        handle_sync();
    }
}
