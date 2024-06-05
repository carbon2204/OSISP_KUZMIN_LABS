CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server
	
client: client.c
	$(CC) $(CFLAGS) client.c -o client


clean:
	rm -f server client
