#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <iostream>

#define MAX_SESSIONS 2

using namespace std;

struct usuario{
    string nome;
    bool sessaoAtiva1;
    bool sessaoAtiva2;
    int socketClient1;
    int socketClient2;
};
typedef struct usuario USUARIO;

#define TAM_LISTA 200

class LoginManager{
    public:
        LoginManager();
        bool login(int socketCli, char user[]);
        string getUser(string user);
        vector<USUARIO> listaDeUsuarios;
        void printListaUsuario();
    private:
        void criarNovoUsuario(string nome,int socketCli);
        bool sessoesAtivas(USUARIO login);
        bool verificaQuantidadeUsuarios(string nome,int socketCli);
};