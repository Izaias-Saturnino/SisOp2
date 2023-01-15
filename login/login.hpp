#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

#define MAX_SESSIONS 2

using namespace std;

struct usuario{
    string nome;
    bool sessaoAtiva1;
    bool sessaoAtiva2;
    struct sockaddr_in servAddr1;
    struct sockaddr_in servAddr2;
};
typedef struct usuario USUARIO;

class LoginManager{
    private:
        void criarNovoUsuario(string nome);
        void sessoesAtivas(USUARIO login);
        void verificaQuantidadeUsuarios(string nome);
    public:
        LoginManager();
        void login();
        vector<USUARIO> listaDeUsuarios;
};