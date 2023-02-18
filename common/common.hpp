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
#include <mutex>
#include <fstream>
#include <vector>

using namespace std;

//messages
#define MENSAGEM_LOGIN 1
#define MENSAGEM_LOGOUT 2
#define MENSAGEM_USUARIO_INVALIDO 3
#define MENSAGEM_USUARIO_VALIDO 4
#define MENSAGEM_RESPOSTA_LOGOUT 5
#define MENSAGEM_ENVIO_NOME_ARQUIVO 10
#define MENSAGEM_ENVIO_PARTE_ARQUIVO 11
#define MENSAGEM_ENVIO_SYNC 12
#define MENSAGEM_ENVIO_TAMANHO_ARQUIVO 13
#define MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR 20
#define MENSAGEM_ITEM_LISTA_DE_ARQUIVOS 21
#define MENSAGEM_DELETAR_NO_SERVIDOR 30
#define MENSAGEM_DELETAR_NOS_CLIENTES 31
#define MENSAGEM_DOWNLOAD_FROM_SERVER 40
#define MENSAGEM_FALHA_ENVIO 41
#define GET_SYNC_DIR 50
#define FIRST_SYNC_END 51
#define ACK 60

//consts
#define BUFFER_SIZE 256

typedef struct {
    uint16_t type; //Tipo do pacote
    uint16_t seqn; //Número de sequência
    char _payload[BUFFER_SIZE]; //Dados do pacote
}PACKET;

struct usuario{
    char nome[BUFFER_SIZE];
    bool sessaoAtiva1;
    bool sessaoAtiva2;
    int socketClient1;
    int socketClient2;
    int sync1;
    int sync2;
};
typedef struct usuario USUARIO;

void serialize(PACKET *pkt, char data[sizeof(PACKET)]);
void deserialize(PACKET *pkt, char data[sizeof(PACKET)]);
int readSocket(PACKET *pkt, int sock);
void sendMessage(char message[BUFFER_SIZE], int messageType, int sockfd);
//returns the amount of bytes written
void receiveFile(int sock, string file_path, PACKET *pkt_addr);
void sendFile(int sock, string file_path);
string getFileName(string file_path);