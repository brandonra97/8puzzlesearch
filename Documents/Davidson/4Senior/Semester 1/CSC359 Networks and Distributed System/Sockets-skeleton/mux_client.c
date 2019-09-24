#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define BUFFER_SIZE 1024

int flush_buffer(int socket, char *buffer, int ntowrite);

// Mux-client: use select()

int main(int argc, char **argv) {
	int result;

	// Argument parsing

	if(argc < 3) {
		fprintf(stderr, "Usage: client <ip> <port>\n");
		return -1;
	}

	char *ip_address = argv[1];
	char *port = argv[2];

	// Using getaddrinfo to obtain the first address to connect to

	struct addrinfo result_hints;
	struct addrinfo *result_list;

	memset(&result_hints, 0, sizeof(struct addrinfo));

	result_hints.ai_family = AF_UNSPEC;
	result_hints.ai_socktype = SOCK_STREAM;

	result = getaddrinfo(ip_address, port, &result_hints, &result_list);

	if(result != 0) {
		perror("Cannot obtain address");

		return -1;
	}

	if(result_list == NULL) {
		fprintf(stderr, "No address found");

		return -1;
	}

	// Socket creation

	int remote_socket;

	remote_socket = socket(result_list->ai_family, result_list->ai_socktype, result_list->ai_protocol);

	if(remote_socket == -1) {
		perror("It wasn't possible to create the socket");

		return -1;
	}

	// Connecting to the server

	result = connect(remote_socket, result_list->ai_addr, result_list->ai_addrlen);

	if(result == -1) {
		perror("Cannot connect to the server");

		return -1;
	}

	char buffer[BUFFER_SIZE];

	fd_set descriptor_set;

	while(1) {
		FD_ZERO(&descriptor_set);

		FD_SET(0, &descriptor_set);
		FD_SET(remote_socket, &descriptor_set);

		result = select(MAX(0,remote_socket) + 1, &descriptor_set, NULL, NULL, 0);

		if(result == -1) {
			perror("select");

			continue;
		}

		// There's something ready from the standard input
		if(FD_ISSET(0, &descriptor_set)) {
			fgets(buffer, BUFFER_SIZE, stdin);

			if(strcmp(buffer, "exit") == 0) {
				break;
			}

			flush_buffer(remote_socket, buffer, strlen(buffer));
		}

		// There's something ready coming from the server
		if(FD_ISSET(remote_socket, &descriptor_set)) {
			int nread = read(remote_socket, buffer, BUFFER_SIZE - 1);

			if(nread == 0) {
				break;
			}

			buffer[nread] = '\0';

			printf("%s", buffer);
		}
	}

	close(remote_socket);

	return 0;
}

int flush_buffer(int socket, char *buffer, int ntowrite) {
	int result;

	int nwritten = 0;

	while(ntowrite > 0) {
		result = write(socket, buffer + nwritten, ntowrite);

		if(result == -1) {
			perror("write");

			return -1;
		}

		nwritten += result;
		ntowrite -= result;
	}

	return nwritten;
}

// No need for receive_string
