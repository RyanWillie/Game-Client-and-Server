//Example code: A simple server side code, which echos back the received message.
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

struct Connect
{
	fd_set readfds;
	int max_clients;
	int PORT;
	int opt;
	int sd;
    int master_socket, addrlen, new_socket, client_socket[30], activity, i, valread;
    int max_sd;
    struct sockaddr_in address;
};


#define TRUE 1
#define FALSE 0

void Connect_Players(struct Connect *PlayerConnections)
{
    int Join = TRUE, i;
	int numPlayers = 0;

    char buffer[1025]; //data buffer of 1K
	char *message = "Welcome to the Game! \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < PlayerConnections->max_clients; i++)
    {
        PlayerConnections->client_socket[i] = 0;
    }

    //create the master socket
    if ((PlayerConnections->master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Master Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    if (setsockopt(PlayerConnections->master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&PlayerConnections->opt,
                   sizeof(PlayerConnections->opt)) < 0)
    {
        perror("Set Master Socket Connections Failed");
        exit(1);
    }

    //type of socket created
    PlayerConnections->address.sin_family = AF_INET;
    PlayerConnections->address.sin_addr.s_addr = INADDR_ANY;
    PlayerConnections->address.sin_port = htons(PlayerConnections->PORT);

    //bind the socket to localhost port 8888
    if (bind(PlayerConnections->master_socket, (struct sockaddr *)&PlayerConnections->address, sizeof(PlayerConnections->address)) < 0)
    {
        perror("bind failed");
        exit(1);
    }
    printf("Now Listening on port %d \n", PlayerConnections->PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(PlayerConnections->master_socket, 3) < 0)
    {
        perror("listen");
        exit(1);
    }

    //accept the incoming connection
    PlayerConnections->addrlen = sizeof(PlayerConnections->address);
    printf("Now waiting for players to connect ...");

	//Code to wait for incomming connections and assign file descriptors
    while(Join)
    {
        //clear the socket set
        FD_ZERO(&PlayerConnections->readfds);

        //add master socket to set
        FD_SET(PlayerConnections->master_socket, &PlayerConnections->readfds);
        PlayerConnections->max_sd = PlayerConnections->master_socket;

        //add child sockets to set
        for (i = 0; i < PlayerConnections->max_clients; i++)
        {
            //socket descriptor
            PlayerConnections->sd = PlayerConnections->client_socket[i];

            //if valid socket descriptor then add to read list
            if (PlayerConnections->sd > 0)
                FD_SET(PlayerConnections->sd, &PlayerConnections->readfds);

            //highest file descriptor number, need it for the select function
            if (PlayerConnections->sd > PlayerConnections->max_sd)
                PlayerConnections->max_sd = PlayerConnections->sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        PlayerConnections->activity = select(PlayerConnections->max_sd + 1, &PlayerConnections->readfds, NULL, NULL, NULL);

        if ((PlayerConnections->activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(PlayerConnections->master_socket, &PlayerConnections->readfds))
        {
            if ((PlayerConnections->new_socket = accept(PlayerConnections->master_socket, (struct sockaddr *)&PlayerConnections->address, (socklen_t *)&PlayerConnections->addrlen)) < 0)
            {
                perror("accept");
                exit(1);
            }

            //printf("New connection, socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            //send new connection greeting message
            if (send(PlayerConnections->new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }
			//add new socket to array of sockets
            for (i = 0; i < PlayerConnections->max_clients; i++)
            {
                //if position is empty
                if (PlayerConnections->client_socket[i] == 0)
                {
                    PlayerConnections->client_socket[i] = PlayerConnections->new_socket;
                    printf("Player %d has entered the game!\n", i + 1);
					numPlayers++;
                    break;
                }
            }

			


        }

        //else its some IO operation on some other socket
        for (i = 0; i < PlayerConnections->max_clients; i++)
        {
           PlayerConnections->sd = PlayerConnections->client_socket[i];

            if (FD_ISSET(PlayerConnections->sd, &PlayerConnections->readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((PlayerConnections->valread = read(PlayerConnections->sd, buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(PlayerConnections->sd, (struct sockaddr *)&PlayerConnections->address,
                                (socklen_t *)&PlayerConnections->addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(PlayerConnections->address.sin_addr), ntohs(PlayerConnections->address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(PlayerConnections->sd);
                    PlayerConnections->client_socket[i] = 0;
                }else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[PlayerConnections->valread] = '\0';
                    send(PlayerConnections->sd, buffer, strlen(buffer), 0);
                }
            }
        }

		if(numPlayers == PlayerConnections->max_clients){
			printf("All players have connected!\r\n");

			//Joining stage is over, loop can exit
			Join = FALSE;
		}
    }
}