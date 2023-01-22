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

void LoginManager::criarNovoUsuario(string nome,int socketCli){
    USUARIO conta;
    conta.nome = nome;
    conta.sessaoAtiva1 = true;
    conta.sessaoAtiva2 = false;
    conta.socketClient1 = socketCli;
    conta.socketClient2 = -1;  // valor invalido
    this->listaDeUsuarios.push_back(conta);
    this->printListaUsuario();
}

bool LoginManager::sessoesAtivas(USUARIO login){
    bool usuarioValido = true;

    if(login.sessaoAtiva1 == true)
    {
        if(login.sessaoAtiva2 == true){
            cout<<"Excedido o número de sessoes possiveis \n"<<endl;
            usuarioValido = false;
        }
        else{
            login.sessaoAtiva2 = true;
            cout<<"Segunda conta = true \n"<<endl;
        }
    }
    else{
        login.sessaoAtiva1 = true;
        cout<<"Primeira conta = true \n"<<endl;
    }

    return usuarioValido;
}

bool LoginManager::verificaQuantidadeUsuarios(string nome,int socketCli){
    vector<USUARIO>::iterator it;
    bool achou = false, usuarioValido = true;
    USUARIO login;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(nome == (*it).nome){
            achou = true;
            login = (*it);
        }
        cout<< it->nome<<endl;
	}

    if(achou != true){
        cout<<"Usuário não encontrado! Criando um novo usuario...\n"<<endl;
        cout<< nome<< endl;
        this->criarNovoUsuario(nome,socketCli);
    }
    else{
        usuarioValido = this->sessoesAtivas(login);
    }

    return usuarioValido;
}

bool LoginManager::login(int socketCli, char user[]){
    bool usuarioValido;

    usuarioValido = this->verificaQuantidadeUsuarios(user,socketCli);
    
    return usuarioValido;
}
