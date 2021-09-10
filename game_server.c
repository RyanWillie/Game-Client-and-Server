//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "connections.h"

#define TRUE 1
#define FALSE 0

void startNumbers(struct Connect *PlayerConnections);

int main(int argc, char *argv[])
{
	//Set the required variables
	struct Connect PlayerConnections;
	PlayerConnections.max_clients = atoi(argv[3]);
	PlayerConnections.PORT = atoi(argv[1]);
	PlayerConnections.opt = TRUE;

	Connect_Players(&PlayerConnections);
	startNumbers(&PlayerConnections);

	return 1;
}

void startNumbers(struct Connect *PlayerConnections){
    char buffer[1025];
	printf("Now we would start the games!\n");
	int playerTurn = 0, connected = PlayerConnections->max_clients;
	int Playing = TRUE;
	//Wait for activity from the players
	
	while(Playing){
		PlayerConnections->activity = select(PlayerConnections->max_sd + 1, &PlayerConnections->readfds, NULL, NULL, NULL);
		printf("i got here!");
		if ((PlayerConnections->activity < 0) && (errno != EINTR))
		{
			printf("select error");
		}

		for (int i = 0; i < PlayerConnections->max_clients; i++)
		{

			PlayerConnections->sd = PlayerConnections->client_socket[i];


			if (FD_ISSET(PlayerConnections->sd, &PlayerConnections->readfds))
			{
				//Client has sent something
				//Check if it was for closing , and also read the incoming message
				if ((PlayerConnections->valread = read(PlayerConnections->sd, buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(PlayerConnections->sd, (struct sockaddr *)&PlayerConnections->address, (socklen_t *)&PlayerConnections->addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(PlayerConnections->address.sin_addr), ntohs(PlayerConnections->address.sin_port));

					//Close the socket and mark as 0 in list for reuse
					close(PlayerConnections->sd);
					PlayerConnections->client_socket[i] = 0;
					printf("A client has disconnected, Game will have to exit\n");
					exit(1);
				}
			}else
				{
					if(playerTurn == i){
						printf("Messaged received from correct player %d\n", playerTurn);
						strcpy(buffer, "It was your turn!\n");
						send(PlayerConnections->sd, buffer, strlen(buffer), 0);
						playerTurn++;
					}else{
						printf("Messaged recieved from incorrect player %d\n", i);
						strcpy(buffer, "It is not your turn!");
						send(PlayerConnections->sd, buffer, strlen(buffer), 0);
					}
				}

		}
	}
}