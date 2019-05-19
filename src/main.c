#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int run_server(uint16_t port);
int run_client(char* servaddr, size_t servaddrlen, uint16_t port);

void print_help(char *argv0)
{
	char help[] =
		"Usage: %s [-h] [-c ADDR] [-p PORT]\n"
		"Simple POSIX socket based chat client and server. "
		"By default, runs in server mode\n"
		"\t-h\tshow this message and exit\n"
		"\t-c ADDR\trun in client mode; connect to server at ADDR address\n"
		"\t-p PORT\tspecify port to host at or server host if running in client mode\n";

	printf(help, argv0);
}

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind, opterr, optopt;

	int opt;

	uint16_t port = 3005;
	uint8_t server_mode = 1;

	char addr[INET6_ADDRSTRLEN];
	size_t addrlen = -1;

	while ((opt = getopt(argc, argv, "hc:p:")) != -1) {
		switch(opt) {
		case 'h':
			print_help(argv[0]);
			break;
		case 'c':
			server_mode = 0;
			strncpy(addr, optarg, INET6_ADDRSTRLEN);
			addrlen = strlen(addr);
			break;
		case 'p':
			break;
		default:
			break;
		}
	}

	if (server_mode) {
		return run_server(port);
	} else {
		return run_client(addr, addrlen, port);
	}

	return -1;
}

