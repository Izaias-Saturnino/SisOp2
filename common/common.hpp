#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <ctime>

using namespace std;

#define MENSAGEM_LOGIN 1
#define MENSAGEM_LOGOUT 2
#define MENSAGEM_USUARIO_INVALIDO 3
#define MENSAGEM_USUARIO_VALIDO 4
#define MENSAGEM_RESPOSTA_LOGOUT 5

typedef struct {
    uint16_t  type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint32_t total_size; //Número total de fragmentos
    uint16_t length; //Comprimento do payload
    char user[256];
    char _payload[256]; //Dados do pacote
}PACKET;

struct usuario{
    char nome[256];
    bool sessaoAtiva1;
    bool sessaoAtiva2;
    int socketClient1;
    int socketClient2;
};
typedef struct usuario USUARIO;

void readSocket(PACKET *pkt, int sock);
void sendMessage(string message, int seqn, int messageType,int fragmentos, char username[], int sockfd);


