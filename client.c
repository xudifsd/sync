#include <inttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include "compress.h"
#include "network.h"
#include "message.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

static char use[] = "sync-client [push|get] <--host server-IP | -h server-IP> [-p port | --port port] <path>";

/**
 * firstly we deflate, only after that can we connect to server, because
 * sometimes the deflate will take much time, and also because the server
 * use the timeout, so after connect we should send file immediately.
 */
static void make_push(int sock, int fd){
	struct stat sb;
	char head[HEAD_LEN];

	if (fstat(fd, &sb) < 0)
		die_on_system_error("fstat");
	if (generate_request_header(PUSH, (uintmax_t)sb.st_size, head, HEAD_LEN) == 0)
		die_on_user_error("can not generate header");

	write_or_die(sock, head, strlen(head));

	copy_between_fd(fd, sock, STDOUT_FILENO, sb.st_size, 1);

	close(sock);
	printf("Successfully send %lld bytes\n", (uintmax_t)sb.st_size);
	exit(0);
}

/**
 * FIXME implement get request maker
 */
static void make_get(int sock, char *path[]){
	exit(0);
}

int main(int argc, char *argv[]){
	char *path = NULL;
	char *ip = NULL;
	int action;
	short port = 0;
	int c;

	int fd, sock;

	if (argc == 1)
		usage(use);
	if (!strcmp(argv[1], "push"))
		action = PUSH;
	else if (!strcmp(argv[1], "get"))
		action = GET;
	else
		usage(use);

	for (;;){
		static struct option long_options[] = {
			{"host", required_argument, NULL, 'h'},
			{"port", required_argument, NULL, 'p'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc - 1, argv + 1, ":h:p:",
				long_options, NULL);
		if (c == -1)
			break;
		switch (c) {
			case 'h':
				ip = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				if (port == 0)
					die_on_user_error("%s is not a valid port", optarg);
				break;
			case '?':
				break;
			case ':':
				die_on_user_error("missing argument\n");
				break;
			default:
				die_on_user_error("getopt returned character code 0%o\n", c);
		}
	}

	path = argv[optind + 1];
	if (path == NULL)
		usage(use);
	if (ip == NULL)
		usage(use);
	if (port == 0)
		port = SERVER_PORT;

	/* deflate before connect */
	if (action == PUSH){
		printf("[%llu] deflating\n", (uintmax_t)getpid());
		fd = deflate(path);
		if (fd < 0)
			die_on_user_error("can not use path %s", path);
	}

	sock = connect_to(ip, port);
	if (sock < 0)
		die_on_user_error("can not connect to %s:%d\n", ip, port);

	if (action == PUSH)
		make_push(sock, fd);
	else if (action == GET)
		make_get(sock, argv);
	else
		usage(use);
	exit(0); /* avoid gcc warning */
}
