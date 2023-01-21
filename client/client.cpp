#include "./client.hpp" 

#define PORT 4000

using namespace std;

int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t clilen;
	char buffer[256], user[256];
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting\n");
	
	bzero(buffer, 256);

	obtemMensagem(buffer,user,sockfd);

	close(sockfd);
	return 0; 
}

void obtemMensagem(char buffer[],char user[],int sockfd){

	cout<<"Por favor insira seu login:"<<endl; //cuidar com espaços
    cin>>user;

	writeSocket(sockfd,user);
	readSocket(buffer,sockfd);
}

void writeSocket(int sockfd, char user[]){
    int n;

	n = write(sockfd, user, strlen(user));
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
