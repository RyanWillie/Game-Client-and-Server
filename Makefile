all: gameserver gameclient gameclientfaulty

gameserver: game_server.o
	gcc game_server.o -o server

gameclient: game_client.o
	gcc game_client.o -o client

gameclientfaulty: game_client_faulty.o
	gcc game_client_faulty.o -o faultyClient

clean:
	rm *.o
	rm server.exe
	rm client.exe
	rm faultyClient.exe

