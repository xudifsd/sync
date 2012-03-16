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

static char use[] = "sync-client push <--host server-IP | -h server-IP> [-p port | --port port] <path>";

int main(int argc, char *argv[]){
	char *path = NULL;
	char *ip = NULL;
	short port = 0;
	int c;

	int fd, sock;
	char head[HEAD_LEN];
	struct stat sb;

	if (argc > 1 && memcmp(argv[1], "push", 4))
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

	printf("[%llu] deflating\n", (uintmax_t)getpid());
	fd = deflate(path);
	if (fd < 0)
		die_on_user_error("can not use path %s", path);
	if (fstat(fd, &sb) < 0)
		die_on_system_error("fstat");

	sock = connect_to(ip, port);
	generate_request_header(PUSH, (uintmax_t)sb. st_size, head, HEAD_LEN);

	write_or_die(sock, head, strlen(head));

	copy_between_fd(fd, sock, STDOUT_FILENO, sb.st_size, 1);

	close(sock);
	printf("Successfully send %s\n", path);
	exit(0);
}
