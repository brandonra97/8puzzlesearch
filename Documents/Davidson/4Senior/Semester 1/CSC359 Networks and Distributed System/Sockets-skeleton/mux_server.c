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
#include <signal.h>
#include <fcntl.h>

#include "ll_double.h"

#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define BUFFER_SIZE 1024

int flush_buffer(int socket, char *buffer, int ntowrite);

void print_address_information(char *template, struct sockaddr *address, int address_size);
int handle_client(int client_socket);

void make_nonblocking(int socket, int flag);
void setup_signal_handler(int signal, void (*handler)(int));

// Ping-pong version 4: use socket multiplexing, one server input for all clients

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

	// If the client closes the connection, the server is signaled with SIGPIPE
	// You don't want that to stop the server, so ignore the signal
	setup_signal_handler(SIGPIPE, SIG_IGN);

	// Start linked list of of clients
	struct list clients;

	ll_init(&clients);

	int maximum_descriptor;
	fd_set set_read;

	// Make the listen_socket non-blocking: that is, do not block if connections are not present
	make_nonblocking(listen_socket, 1);

	while(1) {
		FD_ZERO(&set_read);
		maximum_descriptor = listen_socket;

		FD_SET(listen_socket, &set_read);
		for(struct node *current = clients.head; current != NULL; current = current->next) {
			int current_socket = (int) current->data;

			FD_SET(current_socket, &set_read);

			if(current_socket > maximum_descriptor) {
				maximum_descriptor = current_socket;
			}
		}

		// Blocks until anything can be read from or written to.
		// - If a new connection is made, the accept socket is marked as readable;
		// - If new data comes from an accepted client, its socket is marked as readable;

		result = select(maximum_descriptor + 1, &set_read, NULL, NULL, NULL);

		if (result < 0){
			perror("select");

			return -1;
		}
		// If you are here, some socket is ready to be written or to be read from.
		// You have two things to do:

		// A) Test if the accept socket has been flagged ready for reading. Use the appropriate macro described in select(2).
		//    If so, make the accepted client socket nonblocking, and insert the client socket in the list
		if(FD_ISSET(listen_socket, &set_read)) {
			int client_socket;
			struct sockaddr_storage client_socket_address;
			socklen_t client_socket_size;

			client_socket = accept(listen_socket, (struct sockaddr *) &client_socket_address, &client_socket_size);

			if(client_socket == -1) {
				perror("Cannot accept client");

				return -1;
			}

			print_address_information("Connection from client from [%s] port [%s]\n", (struct sockaddr *) &client_socket_address, client_socket_size);

			// Inserts the client into the list

			make_nonblocking(client_socket, 1);
			ll_insert_tail(&clients, (void *) client_socket);
		}

		// B) Iterate over all currently accepted clients [an example of iteration if given below]
		//    If the client is ready for reading OR ready for writing then call handle_client() to deal with the data
		for(struct node *current = clients.head; current != NULL; current = current->next) {
			int current_socket = (int) current->data;

			if(FD_ISSET(current_socket, &set_read)) {
				if(handle_client(current_socket) == -1) {
					current->data = NULL;
				}
			}
		}

		// Remove dead clients after we process them above
		while(ll_remove(&clients, NULL)) {};
	}

	// Go over all clients and close their sockets
	for(struct node *current = clients.head; current != NULL; current = current->next) {
		int current_socket = (int) current->data;

		close(current_socket);
	}

	return EXIT_SUCCESS;
}

void setup_signal_handler(int signal, void (*handler)(int)) {
	struct sigaction request;

	memset(&request, 0, sizeof(struct sigaction));

	request.sa_handler = handler;

	if(sigaction(signal, &request, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

// Makes socket non-blocking for reading or writing
void make_nonblocking(int socket, int flag) {
	int options;

	options = fcntl(socket, F_GETFL);

	if(options < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}

	fcntl(socket, F_SETFL, flag ? (options | O_NONBLOCK) : (options & (~O_NONBLOCK)));

	if(options < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
}

int handle_client(int client_socket) {
	// Handles client data

	char buffer[BUFFER_SIZE];

	int nread = read(client_socket, buffer, BUFFER_SIZE - 1);

	if(nread == 0) {
		close(client_socket);

		return -1;
	}

	buffer[nread] = '\0';

	printf("%s", buffer);

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
