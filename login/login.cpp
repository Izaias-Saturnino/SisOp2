#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include "./login.hpp"

#define TAM_LISTA 200

using namespace std;

LoginManager::LoginManager(){

}

void LoginManager::criarNovoUsuario(string nome){
    USUARIO conta;
    conta.nome=nome;
    conta.sessaoAtiva1 = true;
    conta.sessaoAtiva2 = false;
    //TO DO : preencher restante da struct quando criar os sockets
    this->listaDeUsuarios.push_back(conta);
}

void LoginManager::sessoesAtivas(USUARIO login){
    if(login.sessaoAtiva1 == true)
    {
        if(login.sessaoAtiva2 == true){
            printf("Excedido o número de sessoes possiveis \n");
            //TO DO:finalizar tentativa de login
        }
        else{
            login.sessaoAtiva2 = true;
            printf("Segunda conta = true \n");
        }
    }
    else{
        login.sessaoAtiva1 = true;
        printf("Primeira conta = true \n");
    }
}

void LoginManager::verificaQuantidadeUsuarios(string nome){
    vector<USUARIO>::iterator it;
    bool achou = false;
    USUARIO login;

    for(it = this->listaDeUsuarios.begin(); it != this->listaDeUsuarios.end(); it++){
        if(nome == (*it).nome){
            achou = true;
            login = (*it);
        }
        cout<< it->nome<<endl;
	}

    if(achou != true){
        printf("Usuário não encontrado! Criando um novo usuario...\n");
        criarNovoUsuario(nome);
    }
    else{
        sessoesAtivas(login);
    }

    printf("fim \n");
}

void LoginManager::login(){
    string login,user2, user3, user4,user5;

    //trocar isso por receber um pacote com username
    cout<<"Por favor insira seu login:"<<endl; //cuidar com espaços
    cin>>login;

    //string to char pointer c++
    verificaQuantidadeUsuarios(login);
    
    user2= "Guto";

    //string to char pointer c++
    verificaQuantidadeUsuarios(user2);

    user3= "Van";
    //string to char pointer c++
    verificaQuantidadeUsuarios(user3);

    user4= "Guto";

    //string to char pointer c++
    verificaQuantidadeUsuarios(user4);

    user5= "Van";

    //string to char pointer c++
    verificaQuantidadeUsuarios(user5);

}

int main(){
    
    LoginManager *loginManager = new LoginManager();
    loginManager->login();

    return 0;
}
