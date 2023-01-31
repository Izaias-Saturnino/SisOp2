#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main(){
    bool server_is_up = open_server();
    if(!server_is_up){
        return 1;
    }

    listen_for_clients();

    while(true){
        bool connection_accepted = accept_connection();
        if(!connection_accepted){
            return 1; // não sei se isso é a melhor maneira de tratar esse erro
        }
        create_connection_handler();
    }
    return 0;
}

create_connection_handler(){
    // cria thread que lida com a conexão separadamente chamando o handle_connection()
}

handle_connection(){
    bool login_successful = handle_login();
    if(!login_successful){
        return 1;
    }

    //bool device_first_sync = ...;
    //if(device_first_sync){
        handle_first_sync();
    //}

    while(true){
        handle_request(); // não se esquecer de mandar mensagens de confirmação para o outro lado sempre que necessário
    }
}

bool handle_login(){
    string request_type = read_request();
    if(request_type != "login"){
        send_message("error");
    }
    else{
        username = ...;
        //password = ...;
        send_message("success");
    }
}

handle_first_sync(){
    bool request_from_new_user = ...;
    //bool new_device = request_from_new_user || ...;

    if(request_from_new_user){
        create_user_folder();
    }

    //versão melhorada do código para versões futuras

    /*
    else if(new_device){
        send_all_files_to_client();
    }

    bool server_client_are_consistent = ...;

    if(!server_client_are_consistent){
        List<string> server_new_files = ...;
        List<string> server_removed_files = ...;

        send_files_to_clients();
        remove_files_on_clients();
    }

    //essa versão melhorada não incluiria o código abaixo
    */

    remove_all_files_from_client();
    send_all_files_to_client();

    send_message("first_sync_end");
}

handle_request(){
    string request_type = read_request();
    //aqui fica todo o código que cuida das requisições de comunicação feitas pelo cliente ao servidor, menos as requisições de login
    switch(request_type){
        case "download_file":
            List<string> args = new List<String>{file, file_path};
            send_message(args);
            break;
        case "delete_file":
            delete_file(file_path);

            List<string> args = new List<String>{"success"};
            send_message(args);

            remove_file_on_clients();
            break;
        case "upload_file":
            create_file_from_request();

            List<string> args = new List<String>{"success"};
            send_message(args);

            send_file_to_clients();
            break;
        case "list_files_from_server":
            string list = list_files_in_sync_dir();

            List<string> args = new List<String>{"list_files_from_server", list};
            send_message(args);
            break;
        case "exit_connection":
            exit_connection();
            break;
        case "could_not_read_request":
            List<string> args = new List<String>{"could_not_read_request"};
            send_message(args);
            break;
        default:
            List<string> args = new List<String>{"error"};
            send_message(args);
            break;
    }
}

send_message(List<string> args){
    string message_type = args[0];
    switch(message_type){
        case "success":
            //TO DO
            break;
        case "file":
            //TO DO
            break;
        case "list_files_from_server":
            //TO DO
            break;
        case "could_not_read_request":
            //TO DO
            break;
        case "first_sync_end":
            //TO DO
            break;
        default:
            //mandar mensagem de erro
            break;
    }
}

remove_file_on_clients(){
    //TO DO
}

send_file_to_clients(){
    //TO DO
}

remove_all_files_on_client(){
    //TO DO
}

send_all_files_to_client(){
    //TO DO
}

//move to dir manager
create_user_folder(){
    //TO DO
}

//move to socket manager
bool open_server(){
    //TO DO
}

//move to socket manager
bool listen_for_clients(){
    //TO DO
}

//move to socket manager
bool accept_connection(){
    //TO DO
}

string read_request(){
    string type;

    bool read_successful = read_from_socket();

    if(!read_successful){
        type = "could_not_read_request";
    }else{
        type = get_message_type();
    }

    return type;
}

//move to socket manager
bool read_from_socket(){
    //se não tiver nada, só retornar false
    //se tiver algo, ler o socket até ter todos os dados necessários para tratar a comunicação
    //quando terminar de obter os dados, gravar os dados em um pacote e retornar true
    //se tiver erro nos dados, a ponto de não conseguir tratar, mandar uma mensagem de erro ao cliente (se possível) e retornar false
}

//move to socket manager
exit_connection(){
    //TO DO
}

get_message_type(){
    //lê a mensagem e retorna o tipo dela
}

//move to dir manager
create_file_from_request(){
    //TO DO
}

//move to dir manager
string list_files_in_sync_dir(){
    //TO DO
}