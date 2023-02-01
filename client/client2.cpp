#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include "./client.hpp" 

using namespace std;

//socket variables
int sockfd;
char buffer[256],username[256];
PACKET receivedPkt;

bool sync_online = false;
bool exit_program = false;

//update handler variables
pthread_t updateThread;

void verificaRecebimentoParametros(int argc){
    if (argc < 3) {  
        cout<<"Faltam parametros"<<endl;
        exit(0);
    }
}

//move to socket manager
bool init_client_connection(int argc, char *argv[]){
    bool connection_successful = true;

    int PORT;
	struct sockaddr_in serv_addr;
	string servAddr;

	strcpy(username, argv[1]);
	PORT = atoi(argv[3]);
	servAddr = string(argv[2]);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		printf("ERROR opening socket");
		connection_successful = false;
	}
        
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	inet_aton(servAddr.c_str(), &serv_addr.sin_addr);
	bzero(&(serv_addr.sin_zero), 8);     

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
		printf("ERROR connecting\n");
		connection_successful = false;
	}

    return connection_successful;
}

//read input and put it in a string
string read_input(){
    string input;
    cin>> input;
    return input;
}

void send_login_request(){
    bzero(buffer, 256);

	sendMessage("",1,1,1,username,sockfd); //login message
}

string read_request(){
    readSocket(&receivedPkt,sockfd);
    return receivedPkt._payload;
}

bool attempt_login(){
    bool successful = true;

    //pegar dados de login
    //string password = ...;

    //envia os dados em uma mensagem de login
    send_login_request();

    string request_answer = read_request();
    if(request_answer != "OK"){
        successful = false;
    }

    return successful;
}

//lida com as mensagens do servidor em uma thread separada
bool create_update_handler(){
    bool update_handler_created = true;
    try{
        pthread_create(&updateThread, NULL, handle_updates, &sockfd);
    }
    catch(exception &e){
        update_handler_created = false;
    }
    return update_handler_created;
}

vector<string> get_args(string const &str, const char delim){
    vector<string> args;

    stringstream ss(str);
 
    string s;
    while (getline(ss, s, delim)) {
        args.push_back(s);
    }

    return args;
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
        //get_file_list("sync_dir");
    }
    else if(type == "get_sync_dir"){
        if(!sync_online){
            sync_online = create_update_handler(); // cria um thread para lidar com atualizações enviadas pelo servidor e cuida dessas atualizações via handle_updates()
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
        //tentar novamente
    }
    else{
        //exibir mensagem de erro
        //pedir para o usuário digitar um comando válido
        //tentar novamente
    }
}

void send_request(){
    string input = read_input();

    vector<string> args = get_args(input, ' ');

    send_request(args);
}

void handle_first_sync(){
    //cria o diretório sync_dir se necessário
    //bool created_user_folder = create_folder(sync_dir_path);
    //if(!created_user_folder){
    //    cerr << "Could not create user folder" << endl;
    //}
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

void* handle_updates(void* sockfd){
    //esperar pelo primeiro pacote do servidor e verificar se ele está pedindo um handle_first_sync()

    handle_first_sync();

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

void exit(){
    //fechar a sessão
    sendMessage("",1,2,1,username,sockfd); //logout message
    //fechar a conexão
    close(sockfd);
    //terminar programa
    exit(0);
}

int main(int argc, char *argv[]){
    verificaRecebimentoParametros(argc);

    bool connection_successful = init_client_connection(argc, argv);
    if(!connection_successful){
        std::cerr << "Connection error" << std::endl;
        exit(0);
    }

    bool login_successful = attempt_login();
    if(!login_successful){
        string message = "";
        cout<< "type exit to end your session \n"<<endl;
		while(message != "exit")
		{
			message = read_input();
		}
        exit();
    }

    while(true){
        send_request();

        if(exit_program){
            exit();
        }
    }
    return 0;
}