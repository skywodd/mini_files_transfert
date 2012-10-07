/* Includes */
#include <stdio.h>      /* For printf() */
#include <string.h>     /* For strncpy() */
#include <sys/types.h>  /* For AF_NET */
#include <sys/socket.h> /* For socket() */
#include <unistd.h>     /* For write() */
#include <netdb.h>      /* For struct hostent */
#include "common.h"     /* For prototypes and error codes */

static int client_send(const int sockfd, const char *filename) {

	/* Local variables */
	unsigned char buffer[BUFFER_SIZE]; /* Output buffer */
	FILE *fi; /* Input file pointer */
	long bytes_to_write, bytes_written, n; /* Numbers of byte to write / written */
	File_header_t file_header; /* Output file header */
	const char *tmp; /* Temp string pointer for strrchr() */

	/* Debug message */
	printf("[DEBUG] Processing file : %s\n", filename);

	/* Debug message */
	puts("[DEBUG] Opening input file ...");

	/* Open file */
	if ((fi = fopen(filename, "rb")) == NULL ) {

		/* Drop error message */
		perror("fopen()");

		/* Return with error */
		return FILE_ERROR;
	}

	/* Compute file header */
	/* Get filename from path */
	if((tmp = strrchr(filename, '/') + 1) == NULL) {
		tmp = filename;
	}

	/* Copy filename to header*/
	strncpy(file_header.name, tmp, FILENAME_SIZE);
	printf("[DEBUG] Relative filename : %s\n", tmp);

	/* Get and store file length */
	fseek(fi, 0, SEEK_END);
	n = ftell(fi);
	fseek(fi, 0, SEEK_SET);
	file_header.length = htonl(n);
	printf("[DEBUG] File length : %ld\n", n);

	/* Debug message */
	puts("[DEBUG] Sending header ...");

	/* Send header */
	if ((bytes_written = write(sockfd, &file_header, sizeof(File_header_t)))
			== -1) {

		/* Drop error message */
		perror("write()");

		/* Return with error */
		return SOCKET_ERROR;
	}

	/* Debug message */
	puts("[DEBUG] Sending data ...");

	/* Send file content */
	/* Initial read */
	bytes_to_write = fread(buffer, 1, BUFFER_SIZE, fi);

	/* Check for end-of-file */
	while (bytes_to_write) {

		/* Debug message */
		/*printf("[DEBUG] Buffering %ld bytes\n", bytes_to_write);*/

		/* Check for file error */
		if (ferror(fi)) {

			/* Drop error message */
			puts("I/O file error !");

			/* Return with error */
			return FILE_ERROR;
		}

		/* Send buffer */
		do {

			/* Send with error check */
			if ((bytes_written = write(sockfd, buffer, bytes_to_write)) == -1) {

				/* Drop error message */
				perror("write()");

				/* Return with error */
				return SOCKET_ERROR;
			}

			/* Repeat send until no more data */
		} while (bytes_to_write - bytes_written > 0);

		/* Next read */
		bytes_to_write = fread(buffer, 1, BUFFER_SIZE, fi);
	}

	/* Return without error */
	return NO_ERROR;
}

int client_mode(const struct hostent *host, const unsigned int port, const char **filenames,
		const int nb_files) {

	/* Local variables */
	int sockfd, err, i; /* Socket fd, error code */
	struct sockaddr_in sa; /* Socket target structure */

	/* Drop welcome message */
	printf("[DEBUG] Connection to %s : %d\n", host->h_name, port);

	/* Compute socket target structure */
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	memcpy(&sa.sin_addr, host->h_addr_list[0], sizeof sa.sin_addr);

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
	puts("[DEBUG] Connection to target host ...");

	/* Connect to target host */
	if (connect(sockfd, (struct sockaddr*) &sa, sizeof sa) == -1) {

		/* Drop error message */
		perror("connect()");

		/* Return with error */
		return SOCKET_ERROR;
	}

	/* Debug message */
	puts("[DEBUG] Starting file process !");

	/* Process each input files */
	for (i = 0; i < nb_files; ++i) {

		/* Process file and check for error */
		if ((err = client_send(sockfd, filenames[i])) != NO_ERROR) {

			/* Return with error */
			return err;
		}
	}

	/* Debug message */
	puts("[DEBUG] Exiting ...");

	/* Close socket */
	close(sockfd);

	/* Return without error */
	return NO_ERROR;
}

