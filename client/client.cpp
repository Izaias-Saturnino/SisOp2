#include "./client.hpp" 
#include "../packet.hpp"

#define PORT 4000

using namespace std;

bool Logout = false;

int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	PACKET pkt;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting\n");
	
	bzero(buffer, 256);

	obtemUsuario(pkt);

	sendMessage("",1,1,1,pkt.user,pkt); //login menssage

	readSocket(buffer,sockfd);

	return 0; 
}

void obtemUsuario(PACKET pkt){   // TO DO: pegar nome do usuario do arg v
	cout<<"Por favor insira seu login:"<<endl; //cuidar com espaços
    cin>>pkt.user;
}

void writeSocket(int sockfd, PACKET pkt){
    int n;

	n = write(sockfd, &pkt, sizeof(pkt));
    if (n < 0) 
		printf("ERROR writing login to socket\n");
}

void readSocket(char buffer[], int sockfd){
    int n;
    /* read from the socket */
    n = read(sockfd, buffer, 256);
    if (n < 0) 
		printf("ERROR reading from socket\n");

    printf("%s\n",buffer);
}

void sendMessage(string message, int seqn, int messageType,int fragmentos, string username, PACKET pkt, int sockfd)
{
	pkt.type = messageType;
	pkt.seqn = seqn;
	pkt.total_size = fragmentos;
	pkt.length = strlen(pkt._payload);
	strcpy(pkt.user, username.c_str());
	strcpy(pkt._payload, message.c_str());

	write(sockfd, &pkt, sizeof(pkt));
}
