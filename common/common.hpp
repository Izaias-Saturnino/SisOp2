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

using namespace std;

#define MENSAGEM_LOGIN 1
#define MENSAGEM_LOGOUT 2
#define MENSAGEM_USUARIO_INVALIDO 3
#define MENSAGEM_USUARIO_VALIDO 4
#define MENSAGEM_RESPOSTA_LOGOUT 5
#define MENSAGEM_ENVIO_NOME_ARQUIVO 10
#define MENSAGEM_ENVIO_PARTE_ARQUIVO 11
#define MENSAGEM_ARQUIVO_LIDO 12
#define MENSAGEM_ENVIO_PARTE_ARQUIVO_SYNC 13
#define MENSAGEM_PEDIDO_LISTA_ARQUIVOS_SERVIDOR 20
#define MENSAGEM_ITEM_LISTA_DE_ARQUIVOS 21
#define MENSAGEM_DELETAR_NO_SERVIDOR 30
#define MENSAGEM_DELETAR_NOS_CLIENTES 31
#define MENSAGEM_DOWNLOAD_NO_SERVIDOR 40
#define MENSAGEM_FALHA_ENVIO 41
#define GET_SYNC_DIR 50
#define FIRST_SYNC_END 51
#define ACK 60
#define BUFFER_SIZE 256

typedef struct {
    uint16_t type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint32_t total_size; //Número total de fragmentos
    uint32_t file_byte_size; //Comprimento do payload
    char user[BUFFER_SIZE];
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
void sendMessage(char message[BUFFER_SIZE], uint32_t file_byte_size, int messageType,int fragmentos, char username[BUFFER_SIZE], int sockfd);


