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

void Connect_Players(struct Connect *PlayerConnections);