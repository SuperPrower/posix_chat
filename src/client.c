#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 	256
#define CLIENT_NAME_MAX	128

int run_client(char *servaddr, size_t servaddrlen, uint16_t port)
{
	// Buffers
	// Buffer for reading from the keyboard
	char write_buf[BUF_SIZE];
	// Buffer for writing to the socket
	char read_buf[BUF_SIZE];

	/*
	int name_len;
	char client_name[CLIENT_NAME_MAX];

	// Get and prepare a name for showing
	printf("Enter your nickname: ");
	scanf("%s", client_name);
	client_name[strlen(client_name) + 1] = 0;
	name_len = strlen(client_name);
	*/

	int sockfd = -1;
	int res;
	struct sockaddr_in6 addr;
	socklen_t addrlen = sizeof(struct sockaddr_in6);

	if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket() failed, errno %s\n", strerror(errno));
		goto cleanup;
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	if (inet_pton(AF_INET6, servaddr, &addr.sin6_addr) < 0) {
		fprintf(stderr, "inet_pton() failed, errno %s\n", strerror(errno));
		goto cleanup;
	}

	if (connect(sockfd, (struct sockaddr *) &addr, addrlen) < 0) {
		fprintf(stderr, "connect() failed, errno %s\n", strerror(errno));
		goto cleanup;
	}

	struct pollfd events[2] = {
		{
			.fd = 0,
			.events = POLLIN
		},
		{
			.fd = sockfd,
			.events = POLLIN
		}
	};

	int g_close = 0;
	while (!g_close) {
		res = poll(events, 2, 1000);
		if (res < 0) {
			// error occured
			fprintf(stderr, "poll() failed, errno %s\n", strerror(errno));
			goto cleanup;
		} else if (res == 0) {
			// timeout
			continue;
		}

		if (events[0].revents & POLLIN) {
			--res;
			events[0].revents = 0;

			int buf_len;
			if ((buf_len = read(0, write_buf, sizeof(write_buf))) < 0) {
				fprintf(stderr, "read() failed, errno %s\n", strerror(errno));
				goto cleanup;
			}

			// Remove newlines
			if (write_buf[buf_len - 1] == '\n') {
				write_buf[buf_len - 1] = 0;
				--buf_len;
			}

			if (strcmp(write_buf, "exit") != 0)
				write(sockfd, write_buf, buf_len + 1);
			else
				g_close = 1;
		}

		if (res && events[1].revents & POLLIN) {
			--res;
			events[1].revents = 0;
		}

	}


cleanup:
	if (sockfd != -1)
		close(sockfd);

	return 0;
}
