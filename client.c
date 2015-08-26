#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{

	// SSL variable
	const SSL_METHOD *ssl_method;
	SSL_CTX *ssl_context;
	SSL *ssl;

	// general variables
	int sock = 0;
	struct sockaddr_in remote;
	char buf[] = "Hello, ssl!";

	// Set up SSL
	SSL_library_init();
	SSL_load_error_strings();

	// Select the type of ssl  we want
	// SSLv23 is the method that allows ALL types of SSL
	// This inlcudes SSLv2, SSLv3, TLSv1, TLSv1.1, and TLSv1.2
	// We will later limit this to just the latter
	ssl_method = SSLv23_client_method();

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

	// Set up cipher suites
	if( SSL_CTX_set_cipher_list(ssl_context, "DEFAULT") == -1){
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// normal connectino setup
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("Failed to create socket");
		return -1;
	}

	remote.sin_family = AF_INET;
	remote.sin_port = htons(443);
	inet_aton("127.0.01", &remote.sin_addr);

	printf("Attempting to connect\n");

	if (connect(sock, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
		perror("Failed to connect");
		return -1;
	}
	// use the context to generate an ssl session
	ssl = SSL_new(ssl_context);

	// check for erros
	if (!ssl) {
		ERR_print_errors_fp(stderr);
		return -1;
	}
	// associate the socket
	SSL_set_fd(ssl, sock);

	printf("Establishing SSL session\n");

	// connect the ssl session
	if (SSL_connect(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
		return -1;
	}

	printf("Sending data\n");

	SSL_write(ssl, buf, strlen(buf));

	SSL_read(ssl, buf, strlen(buf));

	printf("Got echo: %s\n", buf);

	// Close the SSL connection
	SSL_shutdown(ssl);
	SSL_free(ssl);

	// Close the socket
	close(sock);

	return 0;
}
