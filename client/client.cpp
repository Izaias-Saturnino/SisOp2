#include "./client.hpp" 

//#define PORT 4000

using namespace std;

bool Logout = false;

int main(int argc, char *argv[])
{
	int sockfd,PORT = atoi(argv[3]);
	socklen_t clilen;
	char buffer[256],username[256];
	struct sockaddr_in serv_addr, cli_addr;
	PACKET pkt;
	string message;

	strcpy(username, argv[1]);
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting\n");
	
	bzero(buffer, 256);

	sendMessage("",1,1,1,username,pkt,sockfd); //login message

	readSocket(buffer,sockfd);

	cout<< "type exit to end your session \n"<<endl;
	while(message != "exit")
	{
		cin>> message;
	}

	sendMessage("",1,2,1,username,pkt,sockfd); //logout message
	return 0; 
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

void sendMessage(string message, int seqn, int messageType,int fragmentos, char username[], PACKET pkt, int sockfd)
{
	pkt.type = messageType;
	pkt.seqn = seqn;
	pkt.total_size = fragmentos;
	pkt.length = strlen(pkt._payload);
	strcpy(pkt.user, username);
	strcpy(pkt._payload, message.c_str());

	write(sockfd, &pkt, sizeof(pkt));
}
