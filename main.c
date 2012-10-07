/* Includes */
#include <stdio.h>  /* For printf() */
#include <string.h> /* For strcmp() */
#include <netdb.h>  /* For gethostbyname() */
#include "common.h" /* For prototypes and error codes */

/**
 * Program entry point
 *
 * @param[in] argc Number of CLI arguments
 * @param[in] argv Array of CLI arguments
 */
int main(int argc, const char **argv) {

	/* Local variables */
	struct hostent *host; /* host info container */
	unsigned int port; /* Listen on (server) / connect to (client) port number */

	/* Check CLI arguments count */
	if (argc < 4) {

		/* On error print USAGE notice */
		printf(
				"USAGE: %s <mode = client / server> <hostname> <port> [<client::filename> <...>]\n",
				argv[0]);

		/* Return with error */
		return ARGUMENT_ERROR;
	}

	/* Compute port number argument */
	if (sscanf(argv[3], "%u", &port) != 1) {

		/* Drop error message */
		puts("Invalid port number argument !");

		/* Return with error */
		return ARGUMENT_ERROR;
	}

	/* Get host info according specified remote / local address */
	if ((host = gethostbyname(argv[2])) == NULL ) {

		/* Drop error message */
		perror("gethostbyname()");

		/* Return with error */
		return HOSTNAME_ERROR;
	}

	/* Switch to client mode according to the mode argument */
	if (strcmp(argv[1], "client") == 0) {

		/* Start client runtime */
		return client_mode(host, port, argv + 4, argc - 4);

	} else {

		/* Switch to server mode according to the mode argument */
		if (strcmp(argv[1], "server") == 0) {

			/* Start server runtime */
			return server_mode(host, port);

		} else {
			/* Unknown mode */

			/* Drop error message */
			puts("Invalid mode argument !");

			/* Return with error */
			return ARGUMENT_ERROR;
		}

	}

	/* Return without error */
	return NO_ERROR;
}
