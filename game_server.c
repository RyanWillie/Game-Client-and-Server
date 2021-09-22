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

void playNumbersPlayer(int sockfd, int playerNum, int sv, int sv2);
void playNumbersController(int numPlayers, int sv[2], int sv2[2]);
void GameOver(int numPlayers, int winner, int sv, int sv2);
enum{CHILD, PARENT};



// Driver function
int main(int argc, char *argv[])
{
	int sockfd, connfd, len, maxPlayers = 2;
	struct sockaddr_in servaddr, cli;
    int sv[2], sv2[2], PORT;
    char *game = argv[2];
    char msg[MAX];

    PORT = atoi(argv[1]);
    maxPlayers = atoi(argv[3]);
	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) != 0){
		perror("Socket pair error\n");
	}else{
		printf("Created Socket\n");
	}

	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv2) != 0){
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

	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
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
            

        if(fork() == 0){
            // Function for chatting between client and server
    		if(strcmp(game, "numbers") == 0)
            	playNumbersPlayer(connfd, i, sv[CHILD], sv2[CHILD]);
            // After chatting exit the thread
			close(connfd);
            exit(EXIT_SUCCESS);
        }else{
            //Parent process handles the game logic
            printf("I'm the parent!\n");
        }
    }

    //Joing stage is complete, proceed to the Play stage
    if(strcmp(game, "numbers") == 0){
        playNumbersController(maxPlayers, sv, sv2);
    }else {
        printf("Unrecognized game!");
    }
    //Ensure all threads have been closed
	printf("Waiting for all threads to exit\n");
    wait(NULL);
    printf("Server now closing\n");
    exit(EXIT_SUCCESS);
}

void playNumbersController(int numPlayers, int sv[2], int sv2[2]){
    char msg[MAX];
    int playerTurn, temp, score = 0, Playing = 1;
    int Action[3];
	printf("\n\n=============================================\n");
    printf("Starting the game loop here\n");
    while(Playing){
        for(int i=0; i < numPlayers; i++){
            playerTurn = i;
            //printf("Waiting for Player %d\n", i);
            do{
                write(sv2[PARENT], &playerTurn, sizeof(playerTurn));
                read(sv2[PARENT], &temp, sizeof(temp));
                //printf("Player %d is ready to play\n", temp);
            }while(temp != playerTurn);
			strcpy(msg, "PLAY");
			write(sv[PARENT], msg, sizeof(msg));
			//printf("Sending player %d the score %d\n", playerTurn, score);
			write(sv[PARENT], &score, sizeof(score));
			//Wait to get the response from the thread what the player did
            read(sv[PARENT], &Action, sizeof(Action));
            if(Action[0] == 1){
                //Playing has taken the move action
				//printf("Adding %d to the score\n", Action[2]);
                score += Action[2];
				printf("The scored is now %d\n", score);
				if(score >= 30){
					printf("Top Score has been reached!\n");
					GameOver(numPlayers, i, sv[PARENT], sv2[PARENT]);
					Playing = 0;
					break;
				}
            }else if(Action[0] == 2){
                //Player has exited
                printf("Player %d has left the Game!\n", playerTurn);
                if(numPlayers == 2){
                    //Other player has won
					if(playerTurn == 1){
                    	GameOver(numPlayers, 0, sv[PARENT], sv2[PARENT]);
					}else {
						GameOver(numPlayers, 1, sv[PARENT], sv2[PARENT]);
					}
					Playing = 0;
					break;
                }
				else{
					numPlayers -= 1;
				}
				
            }
            printf("From player %d: %s\n", playerTurn, msg);
			printf("\n\n");
        }
    }
}

// Function designed for chat between client and server.
void playNumbersPlayer(int sockfd, int playerNum, int sv, int sv2)
{
	char buff[MAX];
    char msg[MAX];
	char sum[10];
	char temp[10];
	int n, Turn, Action[3], loop = 1;

	strcpy(msg, "Welcome to the game Player ");
	sprintf(temp, "%d", playerNum);
	strcat(msg, temp);
	printf("1Writing to %d, |%s|\n", playerNum, msg);
	write(sockfd, msg, sizeof(msg)); //Send welcome message
	//printf("Sent welcome message for %d\n", playerNum);

	// infinite loop for chat
	while(loop) {
		bzero(buff, MAX);
		//====================================== Check its my turn ==============================================
        do{
			read(sv2, &Turn, sizeof(Turn));
			write(sv2, &playerNum, sizeof(playerNum));
			//printf("Player %d but its %d Turn\n", playerNum, Turn);
            //Controller lets me know whose turn it is
        }while(Turn != playerNum); //It is actually my turn, continue on
		//printf("Player %d thread leaving loop\n", playerNum);
		 //Let main controller process aware i'm ready to talk
		//==================================== Send instructions to player ===========================================;
		read(sv, &buff, sizeof(buff));
		if(strcmp(buff, "PLAY") == 0){
			read(sv, &n, sizeof(n));
			//printf("Player %d Received the score: %d\n", playerNum, n);
			sprintf(sum, "%d", n);
			strcpy(buff, "TEXT the sum is "); 
			strcat(buff, sum);
			//printf("4Writing to %d, |%s|\n", playerNum, buff);
			write(sockfd, &buff, sizeof(buff));
			bzero(buff, MAX);
			strcpy(buff, "GO");
			//printf("5Writing to %d, |%s|\n", playerNum, buff);
			write(sockfd, &buff, sizeof(buff)); 
			printf("Waiting to receive response\n");
			if(read(sockfd, &buff, sizeof(buff)) == -1){
				printf("Client %d has timed out!\n");
				strcpy(buff, "QUIT");
			}
		}else if(strcmp(buff, "END") == 0){
			read(sv, &buff, sizeof(buff));
			write(sockfd, &buff, sizeof(buff));
			strcpy(buff, "END");
			write(sockfd, &buff, sizeof(buff));
			loop=0;
		}	
		
		//=========================================== Process the response ===========================================
        //Tokenise the input buffer
        char *token = strtok(buff, " ");
        //Determine what the player wanted to do
        if(strcmp(token, "MOVE") == 0){
			//printf("They responded with a move!\n");
            token = strtok(NULL, " ");
            Action[0] = 1;
            Action[2] = atoi(token);
            write(sv, &Action, sizeof(Action));
            //Send response back to controller thread
        }else if(strcmp(token, "QUIT") == 0){
            //printf("User wants to quit\n");
            token = strtok(NULL, " ");
            Action[0] = 2;
			Action[1] = playerNum;
			write(sv, &Action, sizeof(Action));
			loop = 0;
        }else{
            printf("Client has sent an invalid message! |%s|\n", token);
			token = strtok(NULL, " ");
            Action[0] = 2;
			Action[1] = playerNum;
			write(sv, &Action, sizeof(Action));
			strcpy(buff, "END");
			write(sockfd, &buff, sizeof(buff));
        }
        bzero(buff, MAX);
	}
	printf("Thread %d now closing\n",playerNum);
}

void GameOver(int numPlayers, int winner, int sv, int sv2){
	int playerTurn, turn;
	char buff[MAX];
	char temp[10];
    for(int i=0; i < numPlayers; i++){
		playerTurn = i; 
		//printf("Waiting for Player %d\n", i);
		do{
			write(sv2, &playerTurn, sizeof(playerTurn));
			read(sv2, &turn, sizeof(turn));
		}while(turn != playerTurn);
		if(playerTurn == winner){
			write(sv, "END", sizeof("END"));
			strcpy(buff, "TEXT Congrats! You won Player  "); 
			sprintf(temp, "%d", winner);
			strcat(buff, temp);
			write(sv, buff, sizeof(buff));
		}else{
			write(sv, "END", sizeof("END"));
			strcpy(buff, "TEXT You Lost! The winner was Player  "); 
			sprintf(temp, "%d", winner);
			strcat(buff, temp);
			write(sv, buff, sizeof(buff));
		}
		
	}
}