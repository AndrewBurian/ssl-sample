all: client server

client: client.c
	gcc -Wall client.c -o client -lssl -lcrypto

server: server.c
	gcc -Wall server.c -o server -lssl -lcrypto
