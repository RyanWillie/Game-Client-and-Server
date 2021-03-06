#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX 100
#define PORT 8080
#define SA struct sockaddr

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
			strcpy(buff, "MOV ");
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
			printf("No idea what you sent! %s\n", token);
		}
	}
	
}

int main()
{
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

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
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
