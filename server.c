/* Includes */
#include <stdio.h>      /* For printf() */
#include <sys/socket.h> /* For socket() */
#include <netdb.h>      /* For struct hostent */
#include <sys/time.h>   /* For timeout */
#include <sys/types.h>  /* For AF_NET */
#include <unistd.h>     /* For write() */
#include <string.h>     /* For memcpy() */
#include "common.h"     /* For prototypes and error codes */

/**
 * Clients table
 */
struct {
	int free, ready, sockfd;
	unsigned long flength;
	FILE* fo;
} clients[NB_MAX_CLIENTS] = { { 1, 0, -1, 0, NULL } };

static int accept_client(const int sockfd) {

	/* Local variables */
	int i;

	/* Debug message */
	/*puts("[DEBUG] New client !");*/

	/* Check all clients table */
	for (i = 0; i < NB_MAX_CLIENTS; ++i) {

		/* Search a free entry */
		if (clients[i].free) {

			/* Debug message */
			printf("[DEBUG] Client accepted in slot %d\n", i);

			/* Client accepted */
			clients[i].free = 0;

			/* Waiting for file header */
			clients[i].ready = 0;

			/* Store socket file descriptor */
			clients[i].sockfd = sockfd;

			/* Return without error */
			return NO_ERROR;
		}
	}

	/* Debug message */
	puts("[DEBUG] Client dropped (no more free place) !");

	/* No free entry found, drop client */
	close(sockfd);

	/* Return without error */
	return NO_ERROR;
}

static int process_client(const int index) {

	/* Local variables */
	unsigned char buffer[BUFFER_SIZE]; /* Input buffer */
	char filename[FILENAME_SIZE]; /* Temp buffer for filename */
	long bytes_received, tmp; /* Number of byte received */
	File_header_t *file_header; /* Pointer to File_header_t */

	/* Debug message */
	/*printf("[DEBUG] New event from client %d\n", index);*/

	/* Check if client send header or data */
	if (!clients[index].ready) { /* header */

		/* Debug message */
		puts("[DEBUG] Receiving header ...");

		/* Receive data */
		if ((bytes_received = read(clients[index].sockfd, buffer,
				sizeof(File_header_t))) == -1) {

			/* Drop error message */
			perror("read()");

			/* Return with error */
			return SOCKET_ERROR;

		}

		if (!bytes_received) { /* no more data */

			/* Debug message */
			/*puts("[DEBUG] Socket closed !");*/

			/* Free client table entry */
			clients[index].free = 1;

			/* Close client socket */
			close(clients[index].sockfd);

			/* Debug message */
			puts("[DEBUG] Slot free !");

			/* Return without error */
			return NO_ERROR;
		}

		/* Turn buffer into file header */
		file_header = (File_header_t*) buffer;

		/* Get file info */
		strcpy(filename, file_header->name);
		printf("[DEBUG] Filename : %s\n", filename);

		/* Get file length */
		clients[index].flength = ntohl(file_header->length);
		printf("[DEBUG] File length : %ld\n", clients[index].flength);

		/* Debug message */
		/*puts("[DEBUG] Opening output file ...");*/

		/* Open output file */
		if ((clients[index].fo = fopen(filename, "wb")) == NULL ) {
			/* Drop error message */
			perror("fopen()");

			/* Return with error */
			return FILE_ERROR;
		}

		/* Debug message */
		puts("[DEBUG] Client ready for data !");

		/* Client is now ready */
		clients[index].ready = 1;

	} else { /* data */

		/* Debug message */
		/*puts("[DEBUG] Receiving data ...");*/

		/* Receive data */
		tmp = (clients[index].flength > BUFFER_SIZE) ?
				BUFFER_SIZE : clients[index].flength;
		if ((bytes_received = read(clients[index].sockfd, buffer, tmp)) == -1) {

			/* Drop error message */
			perror("read()");

			/* Return with error */
			return SOCKET_ERROR;
		}

		/* Process data */
		if (bytes_received) {

			/* Debug message */
			/*puts("[DEBUG] Writing data ...");*/

			/* Write data to output file */
			if (fwrite(buffer, 1, bytes_received, clients[index].fo)
					!= bytes_received) {

				/* Drop error message */
				puts("I/O file error !");

				/* Return with error */
				return FILE_ERROR;
			}

			/* Update file length */
			clients[index].flength -= bytes_received;
			/*printf("[DEBUG] File length remaining : %ld\n",
					clients[index].flength);*/

			/* Check for end-of-file */
			if (clients[index].flength <= 0) {

				/* Debug message */
				puts("[DEBUG] End-of-file !");

				/* Waiting for next file header */
				clients[index].ready = 0;
			}

		} else { /* no more data */

			/* Debug message */
			/*puts("[DEBUG] Socket closed !");*/

			/* Free client table entry */
			clients[index].free = 1;

			/* Close client socket */
			close(clients[index].sockfd);

			/* Close output file */
			fclose(clients[index].fo);

			/* Debug message */
			puts("[DEBUG] Slot free !");
		}

	}

	/* Return without error */
	return NO_ERROR;
}

int server_mode(const struct hostent *host, const unsigned int port) {

	/* Local variables */
	socklen_t addr_length; /* Sockaddr length */
	struct sockaddr_in sa; /* Socket remote target info structure */
	fd_set rfds; /* File descriptor set for select() */
	int i, sockfd, conn_sockfd, maxfd, err; /* listening socket fd, client socket fd, max fd number */

	/* Debug message */
	printf("[DEBUG] Listen on %s : %d\n", host->h_name, port);

	/* Compute socket bind structure */
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr, host->h_addr_list[0], sizeof(sa.sin_addr));

	/* Debug message */
	puts("[DEBUG] Opening socket ...");

	/* Open socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

		/* Drop error message */
		perror("socket()");

		/* Return with error */
		return SOCKET_ERROR;
	}

	/* Debug message */
	puts("[DEBUG] Binding socket ...");

	/* Bind socket to local address */
	if (bind(sockfd, (struct sockaddr*) &sa, sizeof sa) == -1) {

		/* Drop error message */
		perror("bind()");

		/* Return with error */
		return SOCKET_ERROR;
	}

	/* Debug message */
	puts("[DEBUG] Start listening ...");

	/* Start listening on socket */
	if (listen(sockfd, 5) == -1) {

		/* Drop error message */
		perror("listen()");

		/* Return with error */
		return SOCKET_ERROR;
	}

	/* CTRL+C FTW #meGusta */
	for (;;) {

		/* Initialize fd_set and maxfd */
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		maxfd = sockfd;

		/* Add all available clients into fd_set */
		for (i = 0; i < NB_MAX_CLIENTS; ++i) {

			/* If client is available */
			if (!clients[i].free) {

				/* Add it to fd_set */
				FD_SET(clients[i].sockfd, &rfds);

				/* Update maxfd */
				if (clients[i].sockfd > maxfd)
					maxfd = clients[i].sockfd;
			}
		}

		/* Debug message */
		/*puts("[DEBUG] Clients list reloaded !");*/

		switch (select(maxfd + 1, &rfds, NULL, NULL, NULL )) {
		case -1: /* If error */
			/* Drop error message */
			perror("select()");

			/* Return with error */
			return SELECT_ERROR;
			break;

		default: /* Socket event */

			/* If event on listening socket */
			if (FD_ISSET(sockfd, &rfds)) {

				/* Accept new incoming socket */
				if ((conn_sockfd = accept(sockfd, (struct sockaddr*) &sa,
						&addr_length)) == -1) {

					/* Drop error message */
					perror("accept()");

					/* Return with error */
					return SOCKET_ERROR;
				}

				/* Process new incoming socket */
				if ((err = accept_client(conn_sockfd)) != NO_ERROR)
					return err;
			}

			/* Process event on other sockets */
			for (i = 0; i < NB_MAX_CLIENTS; ++i) {

				/* If client is available */
				if (!clients[i].free) {

					/* If event from client */
					if (FD_ISSET(clients[i].sockfd, &rfds)) {

						/* Process client */
						if ((err = process_client(i)) != NO_ERROR)
							return err;
					}
				}
			}
			break;
		}
	}

	/* Close socket */
	close(sockfd);

	/* Return without error */
	return NO_ERROR;
}
