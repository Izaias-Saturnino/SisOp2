#ifndef packet_hpp__
#define packet_hpp__

#include <stdint.h>

typedef struct packet{
    uint16_t  type; //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn; //Número de sequência
    uint32_t total_size; //Número total de fragmentos
    uint16_t length; //Comprimento do payload
    const char* _payload; //Dados do pacote
} packet;

#endif