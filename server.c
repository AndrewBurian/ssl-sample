#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>

int main()
{
	// SSL variables
	const SSL_METHOD *ssl_method;
	SSL_CTX *ssl_context;
	SSL *ssl;
	const char *ciphers = "HIGH";

	// general variables
	int server_sock;
	int client_sock;
	struct sockaddr_in local;
	struct sockaddr_in remote;
	socklen_t socklen = sizeof(struct sockaddr_in);
	int opt = 1;
	char buf[255] = { 0 };
	int read = 0;

	// Set up SSL
	SSL_library_init();
	SSL_load_error_strings();

	// Select the type of ssl  we want
	// SSLv23 is the method that allows ALL types of SSL
	// This inlcudes SSLv2, SSLv3, TLSv1, TLSv1.1, and TLSv1.2
	// We will later limit this to just the latter
	ssl_method = SSLv23_server_method();

	// check for errors
	if (!ssl_method) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	// Use the selected method to create a context
	ssl_context = SSL_CTX_new(ssl_method);

	// check for errors
	if (!ssl_context) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	// Limit CTX to just use TLSv1.2 by blocking the rest
	SSL_CTX_set_options(ssl_context,
			    SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1
			    | SSL_OP_NO_TLSv1_1);

	// Set up certs
	if (SSL_CTX_use_certificate_file
	    (ssl_context, "cert.crt", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	if (SSL_CTX_use_PrivateKey_file
	    (ssl_context, "cert.key", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	// Set up cipher suites
	if (SSL_CTX_set_cipher_list(ssl_context, ciphers) == -1) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	// standard server setup
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!server_sock) {
		perror("Failed to open server socket");
		return -1;
	}

	local.sin_family = AF_INET;
	local.sin_port = htons(443);
	local.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int))
	    == -1) {
		perror("Set sock opt");
		return -1;
	}

	if (bind(server_sock, (struct sockaddr *)&local, socklen) == -1) {
		perror("Bind failed");
		return -1;
	}

	listen(server_sock, 1);

	// main server loop
	printf("Starting server\n");

	while (1) {
		client_sock =
		    accept(server_sock, (struct sockaddr *)&remote, &socklen);
		if (!client_sock) {
			perror("Accept failed");
			continue;
		}
		// new client optained
		printf("Got new connection: %s:%d\n",
		       inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));

		// use the context to generate an ssl session
		ssl = SSL_new(ssl_context);

		// check for erros
		if (!ssl) {
			ERR_print_errors_fp(stderr);
			continue;
		}
		// set the ssl session to use the new client
		SSL_set_fd(ssl, client_sock);

		// Accept a new SSL Session
		if (SSL_accept(ssl) == -1) {
			ERR_print_errors_fp(stderr);
			SSL_free(ssl);
			close(client_sock);
			continue;
		}

		printf("Client established SSL Connection\n");

		// Read client data
		read = SSL_read(ssl, buf, 255);
		if (read > 0) {
			printf("Client encrypted data: %s\n", buf);

			// echo data
			SSL_write(ssl, buf, read);

			// zero buffer
			bzero(buf, 255);
		} else {
			ERR_print_errors_fp(stderr);
		}

		// shut down SSL connection
		SSL_shutdown(ssl);
		SSL_free(ssl);

		// shut down the TCP connection
		close(client_sock);

	}

	return 0;
}
