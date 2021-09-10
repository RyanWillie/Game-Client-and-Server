#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define MAX 100
#define PORT 8080
#define SA struct sockaddr

enum{CHILD, PARENT};

// Function designed for chat between client and server.
void func(int sockfd, int playerNum, int sv)
{
	char buff[MAX];
	int n, Turn;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);
		// read the message from client and copy it in buffer
		read(sockfd, buff, sizeof(buff));
		// print buffer which contains the client contents
        read(sv, &Turn, sizeof(Turn));
        printf("Its %d Turn, and im %d\n", Turn, playerNum);
        if(Turn == playerNum) {
            printf("From client %d: %s\t To client : ", playerNum, buff);
            bzero(buff, MAX);
            n = 0;
            // copy server message in the buffer
            while ((buff[n++] = getchar()) != '\n')
                ;

            // and send that buffer to client
            write(sockfd, buff, sizeof(buff));
            write(sv, buff, sizeof(buff));
            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("Server Exit...\n");
                break;
            }
        }
	}
}

// Driver function
int main()
{
	int sockfd, connfd, len, maxPlayers = 2;
	struct sockaddr_in servaddr, cli;
    int sv[2];
    char msg[MAX];
	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) != 0){
		perror("Socket pair error\n");
	}else{
		printf("Created Socket\n");
	}
	// socket create and verification
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

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

    for(int i=0; i < maxPlayers; i++){
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }
        else
            printf("server acccept the client...\n");

        if(fork() == 0){
            // Function for chatting between client and server
            func(connfd, i, sv[CHILD]);
            // After chatting close the socket
            exit(EXIT_SUCCESS);
        }else{
            printf("I'm the parent!\n");
        }
    }
    printf("Starting the game loop here\n");
    int playerTurn = 0;
    write(sv[PARENT], &playerTurn, sizeof(playerTurn));
    read(sv[PARENT], &msg, sizeof(msg));
    printf("From player %d: %s\n", playerTurn, msg);
    wait(NULL);
}
