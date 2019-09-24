#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// First server: IPv4-only version

int main(int argc, char **argv) {
	int result;

	// Argument parsing

	if(argc < 2) {
		fprintf(stderr, "Usage: server <port>\n");

		return -1;
	}

	char *port = argv[1];

	// Listening socket creation

	int listen_socket;
	struct sockaddr_in address_socket;

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(listen_socket == -1) {
		perror("It wasn't possible to create the socket");

		return -1;
	}

	// Binding to a local address/port

	memset(&address_socket, 0, sizeof(struct sockaddr_in));

	address_socket.sin_family = AF_INET;

	address_socket.sin_port = htons(atoi(port));

	result = bind(listen_socket, (struct sockaddr *) &address_socket, sizeof(struct sockaddr_in));

	if(result == -1) {
		perror("Impossible to bind the server to port");

		return -1;
	}

	// Listen for connections

	result = listen(listen_socket, 5);

	if(result == -1) {
		perror("Impossible to listen to connections");

		return -1;
	}

	// Wait and accept a single client connection

	int client_socket;
	struct sockaddr_in client_socket_address;
	socklen_t client_socket_size;

	client_socket_size = sizeof(struct sockaddr_in);

	client_socket = accept(listen_socket, (struct sockaddr *) &client_socket_address, &client_socket_size);

	if(client_socket == -1) {
		perror("Cannot accept client");

		return -1;
	}

	// Read from client and echo its messages

	char buffer[BUFFER_SIZE];
	int nread;

	while((nread = read(client_socket, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[nread] = '\0';
		printf("%s", buffer);
	}

	printf("The client finished the connection\n");

	return 0;
}
