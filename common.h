#ifndef COMMON_H_
#define COMMON_H_

#include <netdb.h>  /* For struct hostent */

/**
 * Max number of simultaneous clients
 */
#define NB_MAX_CLIENTS 10

/**
 * Buffer size
 */
#define BUFFER_SIZE 4096

/**
 * Error codes
 */
enum {
	NO_ERROR,
	ARGUMENT_ERROR,
	HOSTNAME_ERROR,
	FILE_ERROR,
	SOCKET_ERROR,
	SELECT_ERROR,
	CHROOT_ERROR,
};

/**
 *
 */
#define FILENAME_SIZE 255

/**
 * File header structure
 */
typedef struct {
	char name[FILENAME_SIZE];
	unsigned long length;
	/*unsigned int crc;*/
} File_header_t;

/**
 * Start client runtime
 *
 * @param host Target host
 * @param port Target port
 * @param filenames Files to send
 * @return Error code
 */
int client_mode(const struct hostent *host, const unsigned int port, const char **filenames, const int nb_files);

/**
 * Start server runtime
 *
 * @param host Local bind address
 * @param port Local listening port
 * @return Error code
 */
int server_mode(const struct hostent *host, const unsigned int port);

#endif /* COMMON_H_ */
