all: gameserver gameclient

gameserver: game_server.o
	gcc game_server.c -o server

gameclient: game_client.o
	gcc game_client.c -o client

clean:
	rm *.o
	rm server.exe
	rm client.exe

