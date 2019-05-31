#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ncurses.h>

#include <errno.h>
#include <signal.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// XXX: currently horribly broken. Won't update until key is pressed
// XXX; ^P's are printed all over the place

#define BUF_LEN 	512
#define MSG_LEN 	256
#define CLIENT_NAME_MAX	128


// ew, global variables. Needed for resize handler.
WINDOW *output_win;
WINDOW *input_win;

/* resizer handler, called when the user resizes the window */
void resizeHandler(int sig) {
	// update windows
	// make sure that input line is at 1 line big
	int outh = LINES - 1;
	outh = outh <= 0 ? 0 : outh;

	wresize(output_win, outh, COLS);
	wresize(input_win, 1, COLS);
}

int run_client(char *servaddr, size_t servaddrlen, uint16_t port)
{
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

	struct pollfd events[1] = { {
		.fd = sockfd,
		.events = POLLIN
	} };

	/* Init ncurses */
	initscr(); raw(); noecho();

	int outh = LINES - 1;
	outh = outh <= 0 ? 0 : outh;

	// terminal dimensions
	output_win = newwin(outh, COLS, 0, 0);
	input_win = newwin(1, COLS, LINES-1, 0);

	scrollok(output_win, TRUE);
	scrollok(input_win, TRUE);

	// enable timeout for reads to be non-blocking
	wtimeout(input_win, 25);

	// add resize signal handler
	signal(SIGWINCH, resizeHandler);

	// user input buffer
	char input_buf[MSG_LEN];
	memset(input_buf, 0, MSG_LEN);
	size_t input_len = 0;

	// Buffer for receiving messages
	char recv_buf[BUF_LEN];
	memset(recv_buf, 0, BUF_LEN);

	int g_close = 0;
	int g_input = 0;

	while (!g_close) {
		/* Poll for user input */
		int ch = wgetch(input_win);
		switch (ch) {
		case ERR:
			// probably a timeout
			break;
		case KEY_ENTER:
		case 10:
			wclear(input_win);
			g_input = 1;
			break;
		case 127:
		case KEY_BACKSPACE: // backspace
			wprintw(input_win, "\b \b");
			input_buf[input_len--] = '\0';
			break;
		default:
			wprintw(input_win, "%c", ch);
			if (input_len < MSG_LEN)
				input_buf[input_len++] = ch;
			break;
		}

		/* Poll socket events */
		res = poll(events, 1, 25);

		if (res < 0) {
			// error occured
			fprintf(stderr, "poll() failed, errno %s\n", strerror(errno));
			goto curses_cleanup;
		} else if (res == 0) {
			// timeout
		} else if (events[0].revents & POLLIN) {
			// TODO: Checks for res == 0 should probably be removed
			--res;
			events[0].revents = 0;

			int bread = read(events[0].fd, recv_buf, BUF_LEN);
			if (bread < 0) {
				fprintf(stderr, "read() failed for socket %d, errno %s\n",
					events[0].fd, strerror(errno)
				);
				goto curses_cleanup;

			} else if (bread == 0) {
				fprintf(stderr, "Server closed the connection\n");
				goto curses_cleanup;

			} else {
				wprintw(output_win, recv_buf);
				memset(recv_buf, 0, BUF_LEN);
			}
		}

		refresh();
		wrefresh(output_win);
		wrefresh(input_win);

		/* send user input */
		if (!g_input)
			continue;

		g_input = 0;

		if (input_buf[0] == '/') {
			// process command
			if (strcmp(input_buf, "/exit") == 0) {
				g_close = 1;
			} else {
				wprintw(output_win, "POSIX_CHAT: %s: Unknown command\n\r", input_buf);
			}

		} else {
			write(sockfd, input_buf, input_len + 1);
		}

		memset(input_buf, 0, MSG_LEN);
		input_len = 0;


	}

curses_cleanup:
	delwin(input_win);
	delwin(output_win);
	endwin();

cleanup:
	if (sockfd != -1)
		close(sockfd);

	return 0;
}
