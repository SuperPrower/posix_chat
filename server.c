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

#define BACKLOG_SIZE	5
#define OPEN_MAX	128

int run_server(uint16_t port)
{
	int sockfd;

	if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "errno %s\n", strerror(errno));
		perror("Unable to open socket\n");
		goto cleanup;
	}

	struct sockaddr_in6 servaddr, cliaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in6);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_port = htons(port);
	servaddr.sin6_addr = in6addr_any;

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		fprintf(stderr, "Unable to bind a name to a socket, errno %s\n", strerror(errno));
		goto cleanup;
	}

	if (listen(sockfd, BACKLOG_SIZE) < 0) {
		fprintf(stderr, "listen() failed, errno %s\n", strerror(errno));
		goto cleanup;
	}

	puts("Server ready for client connect()");

	/* Poll for new connections using poll() */

	struct pollfd clients[OPEN_MAX];
	clients[0].fd = sockfd;
	clients[0].events = POLLIN;

	// initialize pollfd structures that will hold client connections
	for (int i = 1; i < OPEN_MAX; i++) {
		clients[i].fd = -1;
		clients[i].events = POLLIN;
	}

	int g_close = 0;
	// Index at which last client FD is stored in pollfd array.
	// Useful for quick insertion and iteration on entries there.
	int lci = 0;
	int res;

	char buf[256];

	while (!g_close) {
		res = poll(clients, lci + 1, 1000);
		if (res < 0) {
			// error occured
			fprintf(stderr, "poll() failed, errno %s\n", strerror(errno));
			goto cleanup;
		} else if (res == 0) {
			// timeout
			continue;
		}

		// res now stores amount of events that has occured
		if (clients[0].revents & POLLIN) {
			// new connection
			res--;
			clients[0].revents = 0;

			if (lci == OPEN_MAX) {
				fprintf(stderr, "Unable to accept new connection\n");
				break;
			}

			int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &addrlen);
			if (connfd < 0) {
				fprintf(stderr, "accept() failed, errno %s\n", strerror(errno));
				goto cleanup;
			}
			char addr[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &cliaddr.sin6_addr.s6_addr, addr, INET6_ADDRSTRLEN);
			printf("New connection on socket %d from %s\n", connfd, addr);

			clients[++lci].fd = connfd;
		}

		// iterate over clients
		for (int c = 1; c <= lci && res > 0; c++) {
			if (clients[c].fd < 0) continue;
			if ((clients[c].revents & (POLLIN | POLLERR)) == 0) continue;

			res--;
			clients[c].revents = 0;

			int bread = read(clients[c].fd, buf, 255);
			if (bread < 0) {
				if (errno == ECONNRESET)
					printf("Client at socket %d has reset the connection\n",
						clients[c].fd
					);
				else
					fprintf(stderr, "read() failed for socket %d, errno %s\n",
						clients[c].fd, strerror(errno)
					);

			} else if (bread == 0) {
				printf("Client at socket %d has disconnected\n", clients[c].fd);

			} else {
				printf("Got message %s from socket %d\n", buf, clients[c].fd);
				// broadcast message
				// TODO

				continue;
			}

			/* close the connection if there was no message */
			close(clients[c].fd);
			clients[c].fd = -1;

			// replase closed pollfd with one at lci if needed
			if (c < lci && lci > 1) {
				clients[c] = clients[lci];
				clients[lci].fd = -1;
			}

			lci--;
		}

	}

cleanup:
	if (sockfd != -1)
		close(sockfd);

	return 0;
}
