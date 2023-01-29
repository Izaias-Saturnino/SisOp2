#include "./login.hpp"

using namespace std;

LoginManager::LoginManager(){

}

void LoginManager::printListaUsuario(){
    vector<USUARIO>::iterator it;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        cout<< it->nome<<endl;
	}
}

void LoginManager::criarNovoUsuario(char nome[],int socketCli){
    USUARIO conta;
    cout<< "dentro do cria novo: " << nome << endl;
    strcpy(conta.nome,nome);
    conta.sessaoAtiva1 = true;
    conta.sessaoAtiva2 = false;
    conta.socketClient1 = socketCli;
    conta.socketClient2 = -1;  // valor invalido
    this->listaDeUsuarios.push_back(conta);
    this->printListaUsuario();
}

void LoginManager::Logout(char user[],int socket){
    vector<USUARIO>::iterator it;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(strcmp(user,(*it).nome) == 0 ){         
            if((*it).socketClient1 == socket)
            {
                (*it).sessaoAtiva1 = false;
            }
            else{
                (*it).sessaoAtiva2 = false;
            }
            
            if((*it).sessaoAtiva1 == false && (*it).sessaoAtiva2 == false) //sem conexão -> remove da lista
            {
                this->listaDeUsuarios.erase(it);
            }
        }
    }
}

bool LoginManager::verificaQuantidadeUsuarios(char nome[],int socketCli){
    vector<USUARIO>::iterator it;
    bool achou = false, usuarioValido = true;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(strcmp(nome,(*it).nome) == 0 ){
            achou = true;
            if((*it).sessaoAtiva1 == true)
            {
                if((*it).sessaoAtiva2 == true){
                    cout<<"Excedido o número de sessoes possiveis \n"<<endl;
                    usuarioValido = false;
                }
                else{
                    cout<<(*it).sessaoAtiva2<<endl;
                    (*it).sessaoAtiva2 = true;
                    cout<<"Segunda conta = true \n"<<endl;
                }
            }
            else{
                (*it).sessaoAtiva1 = true;
                cout<<"Primeira conta = true \n"<<endl;
            }
        }
        cout<< (*it).nome<<endl;
	}

    if(achou != true){
        cout<<"Usuário não encontrado! Criando um novo usuario...\n"<<endl;
        this->criarNovoUsuario(nome,socketCli);
    }

    return usuarioValido;
}

bool LoginManager::login(int socketCli, char user[]){
    bool usuarioValido;

    usuarioValido = this->verificaQuantidadeUsuarios(user,socketCli);
    
    return usuarioValido;
}
