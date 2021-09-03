all: gameserver

gameserver: game_server.o
	gcc game_server.o connections.c -o gameserver

game_server.o: game_server.c
	gcc game_server.c -c

clean:
	rm *.o
