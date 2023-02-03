#include "./login.hpp"

using namespace std;

mutex mtx_list; //evitar problemas com a lista de usuarios
mutex mtx_sessoes; // evitar problemas com sessoes ativas

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

    strcpy(conta.nome,nome);
    conta.sessaoAtiva1 = true;
    conta.sessaoAtiva2 = false;
    conta.socketClient1 = socketCli;
    conta.socketClient2 = -1;  // valor invalido

    mtx_list.lock();
    this->listaDeUsuarios.push_back(conta);
    this->printListaUsuario();
    mtx_list.unlock();
}

void LoginManager::Logout(char user[],int socket, char resposta[]){
    vector<USUARIO>::iterator it;
    
    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(strcmp(user,(*it).nome) == 0 ){  
            mtx_sessoes.lock();       
            if((*it).socketClient1 == socket)
            {
                (*it).sessaoAtiva1 = false;
                strcpy(resposta,"Sessao 1 desconectada");
            }
            else{
                (*it).sessaoAtiva2 = false;
                strcpy(resposta,"Sessao 2 desconectada");
            }
            break;
            mtx_sessoes.unlock();
        }
    }
    if((*it).sessaoAtiva1 == false && ((*it).sessaoAtiva2 == false)) //sem conexão -> remove da lista
    {
        // mtx_list.lock();
        // this->listaDeUsuarios.erase(it);
        // mtx_list.unlock();
        strcpy(resposta,"Usuario desconectado");
    }
    
}

bool LoginManager::verificaQuantidadeUsuarios(char nome[],int socketCli){
    vector<USUARIO>::iterator it;
    bool achou = false, usuarioValido = true;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(strcmp(nome,(*it).nome) == 0 ){
            achou = true;
            mtx_sessoes.lock();
            if((*it).sessaoAtiva1 == true)
            {
                if((*it).sessaoAtiva2 == true){
                    cout<<"Excedido o número de sessoes possiveis \n"<<endl;
                    usuarioValido = false;
                }
                else{
                    cout<<(*it).sessaoAtiva2<<endl;
                    (*it).sessaoAtiva2 = true;
                    (*it).socketClient2 = socketCli;
                    cout<<"sessao2:"<< (*it).sessaoAtiva2<< endl;
                    cout<<"Segunda conta = true \n"<<(*it).socketClient2<<endl;
                }
            }
            else{
                 if((*it).sessaoAtiva2 == true){ //reativa sessão 1
                    cout<<(*it).sessaoAtiva1<<endl;
                    (*it).sessaoAtiva1 = true;
                    (*it).socketClient1 = socketCli;
                    cout<<"sessao1:"<< (*it).sessaoAtiva1<< endl;
                    cout<<"Segunda conta = true \n"<<(*it).socketClient1<<endl;
                 }
                 else{
                    (*it).sessaoAtiva1 = true;
                    cout<<"Primeira conta = true \n"<<endl;
                 }
            }
            mtx_sessoes.unlock();
        }
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
