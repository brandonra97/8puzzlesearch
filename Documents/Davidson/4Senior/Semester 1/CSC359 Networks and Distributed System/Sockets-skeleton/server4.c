#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Fourth server: denial of service potential

void print_address_information(char *template, struct sockaddr *address, int address_size);

int receive_string(int socket, char *buffer, int maximum);

int main(int argc, char **argv) {
	int result;

	// Argument parsing

	if(argc < 2) {
		fprintf(stderr, "Usage: server <port>\n");

		return -1;
	}

	char *port = argv[1];

	// Using getaddrinfo to obtain the first address to bind to

	struct addrinfo result_hints;
	struct addrinfo *result_list;

	memset(&result_hints, 0, sizeof(struct addrinfo));

	result_hints.ai_family = AF_UNSPEC;
	result_hints.ai_socktype = SOCK_STREAM;
	result_hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, port, &result_hints, &result_list);

	if(result != 0) {
		perror("Cannot obtain address");

		return -1;
	}

	// Listening socket creation

	int listen_socket;

	for(struct addrinfo *result_curr = result_list; result_curr != NULL; result_curr = result_curr->ai_next) {
		// Listening socket creation

		listen_socket = socket(result_curr->ai_family, result_curr->ai_socktype, result_curr->ai_protocol);

		if(listen_socket == -1) {
			continue;
		}

		// Binding to a local address/port

		result = bind(listen_socket, result_curr->ai_addr, result_curr->ai_addrlen);

		if(result == -1) {
			close(listen_socket);
			listen_socket = -1;

			continue;
		}

		print_address_information("Listening in address [%s] port [%s]\n", result_curr->ai_addr, result_curr->ai_addrlen);

		break;
	}

	if(listen_socket == -1) {
		fprintf(stderr, "Not possible to bind to any address/port\n");

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
	struct sockaddr_storage client_socket_address;
	socklen_t client_socket_size;

	client_socket_size = sizeof(struct sockaddr_storage);

	client_socket = accept(listen_socket, (struct sockaddr *) &client_socket_address, &client_socket_size);

	if(client_socket == -1) {
		perror("Cannot accept client");

		return -1;
	}

	// Read from client and echo its messages
	print_address_information("Connection from client from [%s] port [%s]\n", (struct sockaddr *) &client_socket_address, client_socket_size);
	handle_client(client_socket);

	printf("The client finished the connection\n");
	close(client_socket);

	return 0;
}

int handle_client(int client_socket) {
	// Read from client and echo its messages

	char buffer[BUFFER_SIZE];
	int nread;

	// Not really a good idea but illustrates a point
	// Can you tell why it's not a good idea?
	while(receive_string(client_socket, buffer, BUFFER_SIZE - 1) > 0) {
		printf("%s", buffer);
	}

	return 0;
}

int receive_string(int socket, char *buffer, int maximum) {
	int result;

	int nread = 0;

	do {
		result = read(socket, buffer + nread, BUFFER_SIZE - nread);

		if(result == -1) {
			perror("read");

			return -1;
		}

		nread += result;
	} while(nread < maximum && buffer[nread - 1] != '\0');

	return nread;
}

void print_address_information(char *template, struct sockaddr *address, int address_size) {
	int result;

	char host[1024];
	char port[16];

	result = getnameinfo(address, address_size, host, 1024, port, 16, NI_NUMERICHOST | NI_NUMERICSERV);

	if(result != 0) {
		perror("Error obtaining information from client");
	}

	printf(template, host, port);
}
