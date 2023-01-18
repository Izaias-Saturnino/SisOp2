#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h> 
#include <iostream>
#include "./server.hpp"

int main(int argc, char *argv[])
{
    int sockfd,newSockfd;
    struct sockaddr_in serv_addr,cli_addr;
    struct hostent *server;
	pthread_t clientThread;
    socklen_t clilen;

    verificaRecebimentoIP(argc, argv);

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        imprimeServerError();
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)  ///Verifica IP valido
        printf("ERROR opening socket\n");
        
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(PORT);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);     
     
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		printf("ERROR on binding\n");
	else
	{	
        while(true)
        {    
		    listen(sockfd, 5);// listen to the clients
            clilen = sizeof(struct sockaddr_in);
            if ((newSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
                printf("ERROR on accept");
            //memset(&package, 0, sizeof(package));
            //thread_mtx.lock();
            std::cout << sockfd << std::endl;
            pthread_create(&clientThread, NULL, Thread, &newSockfd);        //CUIDADOO : newSocket e não socket
                                              //ponteiro para função, ponteiro para parametro        
        }
    }

    close(sockfd);

    return 0;
}

void verificaRecebimentoIP(int argc,char *argv[]){
    if (argc < 2) {  //Verifica se recebeu o IP como parametro
        fprintf(stderr,"usage %s hostname\n", argv[0]);
        exit(0);
    }
}

void imprimeServerError(void){
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
}

void obtemMensagem(char buffer[]){
    printf("Enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);
    printf("%s\n",buffer);
}

void writeSocket(char buffer[],int sockfd){
    int n;
    /* write in the socket */
    std::cout << sockfd << std::endl;
    std::cout << buffer << std::endl;
	n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
		printf("ERROR writing to socket\n");

    std::cout<<"lele"<<std::endl;
    bzero(buffer,256);
    printf("%s\n",buffer);
}

void readSocket(char buffer[], int sockfd){
    int n;
    /* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0) 
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);
}

void *Thread(void *arg) {
    int sockfd= *(int *) arg;
    char buffer[256]="lele";
    int n;

    //obtemMensagem(buffer);
    std::cout << sockfd << std::endl;
    writeSocket(buffer,sockfd);
    
    readSocket(buffer,sockfd);

    return 0;
}
