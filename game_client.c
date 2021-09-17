#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include<arpa/inet.h>

#define MAX 100
#define SA struct sockaddr

int hostname_to_ip(char * hostname , char* ip);
void func(int sockfd);

int main(int argc, char *argv[])
{
	int PORT = atoi(argv[3]);
	char *serverName = argv[2];
	char *gameType = argv[1];
	char IP[MAX];
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	//Get the IP of the hostname
	hostname_to_ip(serverName, IP);
	printf("The Server IP is %s\n", IP);
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP);
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}


int hostname_to_ip(char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( hostname ) ) == NULL) 
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	
	return 1;
}

void func(int sockfd)
{
	char buff[MAX];
	int n;
	char MOVE[MAX];
	char temp[3];
	bzero(buff, sizeof(buff));
	//Get the welcome message
	//printf("Getting welcome message\n");
	printf("\n\n=============================================\n");
	read(sockfd, buff, sizeof(buff));
	printf("          %s\n", buff);
	
	bzero(buff, sizeof(buff));
	while(1) {
		bzero(buff, sizeof(buff));
		//printf("Waiting for message\n");
		read(sockfd, buff, sizeof(buff)); //Wait until its my Turn
		//printf("I received a messge %s\n", buff);
		//printf("I received: %s\n", buff);
		char *token = strtok(buff, " ");
		//printf("Tokenised buffer\n");
		if(strcmp(token, "GO") == 0){
			bzero(buff, sizeof(buff));
			printf("GO: ");
			scanf("%i", &n);
			strcpy(buff, "MOVE ");
			sprintf(temp, "%d", n);
			strcat(buff, temp);
			//printf("       Writing to server |%s|\n", buff);
			write(sockfd, buff, sizeof(buff));
			bzero(buff, sizeof(buff));
		}else if(strcmp(token, "TEXT") == 0 ){
			while( token != NULL ) {
				printf( "%s ", token );
				token = strtok(NULL, " ");
			}
			printf("\n");
		}else if((strcmp(token, "ERROR")) == 0){
			while( token != NULL ) {
				printf( "%s ", token );
				token = strtok(NULL, " ");
			}
			printf("\n");
		}else if ((strcmp(token, "END")) == 0) {
			printf("Game has ended!\n");
			break;
		}else{
			printf("Invalid message sent from Server! %s\n", token);
			printf("Game will now exit\n");
			break;
		}
	}
	
}