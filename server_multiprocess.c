#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define MAX 100
#define SA struct sockaddr

void playNumbers(int numPlayers, int sv[2]);
enum{CHILD, PARENT};

// Function designed for chat between client and server.
void func(int sockfd, int playerNum, int sv)
{
	char buff[MAX];
	int n, Turn, Action[3];
	// infinite loop for chat
	while(1) {
		bzero(buff, MAX);
		read(sockfd, buff, sizeof(buff)); //This blocks until the player sends a message
        write(sv, &playerNum, sizeof(Turn)); //Let main controller process aware i'm ready to talk
        read(sv, &Turn, sizeof(Turn)); //Controller lets me know whose turn it is
        printf("Its %d Turn, and im %d\n", Turn, playerNum);
        if(Turn == playerNum) {
            //It is actually my turn, continue on
            printf("From client %d: %s\n", playerNum, buff);            
            //Tokenise the input buffer
            char *token = strtok(buff, " ");
            //Determine what the player wanted to do
            if(strcmp(token, "MOVE") == 0){
                token = strtok(NULL, " ");
                Action[0] = 1;
                Action[2] = atoi(token);
                write(sv, &Action, sizeof(Action));
                //Send response back to controller thread
            }else if(strcmp(token, "QUIT") == 0){
                printf("User wants to quit\n");
                write(sv, "QUIT", sizeof("QUIT"));
                strcpy(buff, "You want to quit");
            }else{
                strcpy(buff, "ERROR");
            }

            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("Closing player socket...\n");
                break;
            }
        }
        bzero(buff, MAX);
	}
}

// Driver function
int main(int argc, char *argv[])
{
	int sockfd, connfd, len, maxPlayers = 2;
	struct sockaddr_in servaddr, cli;
    int sv[2], PORT;
    char temp;
    char msg[MAX];

    PORT = atoi(argv[1]);
    maxPlayers = atoi(argv[3]);
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
            printf("server acccept the client %d...\n", i);
            strcpy(msg, "Welcome to the game Player ");
            sprintf(temp, "%c", i);
            strcat(msg, temp);
            write(connfd, msg, sizeof(msg)); //Send welcome message

        if(fork() == 0){
            // Function for chatting between client and server
            func(connfd, i, sv[CHILD]);
            // After chatting exit the thread
            exit(EXIT_SUCCESS);
        }else{
            //Parent process handles the game logic
            printf("I'm the parent!\n");
        }
    }

    //Joing stage is complete, proceed to the Play stage
    char *game = argv[2];
    if(strcmp(game, "numbers") == 0){
        playNumbers(maxPlayers, sv);
    }else {
        printf("Unrecognized game!");
    }
    //Ensure all threads have been closed
    wait(NULL);
    printf("Server now closing\n");
    exit(EXIT_SUCCESS);
}

void playNumbers(int numPlayers, int sv[2]){
    char msg[MAX];
    int playerTurn, temp, score = 0, Playing = 1;
    int Action[3];
    printf("Starting the game loop here\n");
    while(Playing){
        for(int i=0; i < numPlayers; i++){
            playerTurn = i;
            printf("Waiting for Player %d\n", i);
            do{
                read(sv[PARENT], &temp, sizeof(temp));
                write(sv[PARENT], &playerTurn, sizeof(playerTurn));
                printf("Player %d wants to talk\n", temp);
            }while(temp != playerTurn);
            read(sv[PARENT], &Action, sizeof(Action));
            if(Action[0] == 1){
                //Playing has taken the move action
                score += Action[2];
            }else if(Action[0] == 2){
                //Player has exited
                printf("Player %d has left the Game!\n", playerTurn);
                Playing = 0;
                if(numPlayers == 2){
                    //Other player has won
                    GameOver(numPlayers, sv);
                }
            }
            printf("From player %d: %s\n", playerTurn, msg);

        }
    }
}

void GameOver(int numPlayers, int winner, int sv){
    
}