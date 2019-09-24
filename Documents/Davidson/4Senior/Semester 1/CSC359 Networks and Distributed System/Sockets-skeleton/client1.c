#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// First client: IPv4-only version

int main(int argc, char **argv) {
	int result;

	// Argument parsing

	if(argc < 3) {
		fprintf(stderr, "Usage: client <ip> <port>\n");
		return -1;
	}

	char *ip_address = argv[1];
	char *port = argv[2];

	// Socket creation

	int remote_socket;
	struct sockaddr_in address_socket;

	remote_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(remote_socket == -1) {
		perror("It wasn't possible to create the socket");

		return -1;
	}

	// Defining where to connect to

	memset(&address_socket, 0, sizeof(struct sockaddr_in));

	address_socket.sin_family = AF_INET;

	result = inet_pton(AF_INET, ip_address, &address_socket.sin_addr);

	if(result <= 0) {
		perror("Could not parse the IP address");

		return -1;
	}

	address_socket.sin_port = htons(atoi(port));

	// Connecting to the server

	result = connect(remote_socket, (struct sockaddr *) &address_socket, sizeof(struct sockaddr_in));

	if(result == -1) {
		perror("Cannot connect to the server");

		return -1;
	}

	char buffer[1024];

	while(1) {
		printf("Type your message to the server: ");
		fgets(buffer, BUFFER_SIZE, stdin);

		write(remote_socket, buffer, strlen(buffer)); // You like to live dangerously
	}

	return 0;
}
