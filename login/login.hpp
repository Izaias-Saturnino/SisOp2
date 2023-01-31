#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <iostream>
#include "../common/common.hpp"

#define MAX_SESSIONS 2

#define TAM_LISTA 200

class LoginManager{
    public:
        LoginManager();
        bool login(int socketCli, char user[]);
        vector<USUARIO> listaDeUsuarios;
        void printListaUsuario();
        void Logout(char user[], int socket,char resposta[]);
    private:
        void criarNovoUsuario(char nome[],int socketCli);
        bool verificaQuantidadeUsuarios(char nome[],int socketCli);
};