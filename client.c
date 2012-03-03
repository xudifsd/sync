#include <arpa/inet.h>
#include <inttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include "compress.h"
#include "transport.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

static char use[] = "sync-client push <--host server-IP | -h server-IP> [-p port | --port port] <path>";

static int connect_to(const char *ip, short port){
	int sock;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		die_on_user_error("unknow IP: %s\n", ip);
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		die_on_system_error("socket");
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die_on_system_error("connect");
	return sock;
}

/* FIXME finally this should change to copy_fd*/
static void send_fd_or_die(int sock, int fd, off_t size){
	char buf[BUFSIZ];
	off_t transported = 0;
	ssize_t nr = 0;
	while (transported < size){
		nr = xread(fd, buf, BUFSIZ);
		if (nr < 0)
			die_on_system_error("read");
		else if (nr == 0)
			break;
		write_or_die(sock, buf, nr);
		transported += nr;
	}
	if (transported != size)
		die_on_user_error("transport failed");
	return;
}

int main(int argc, char *argv[]){
	char *path = NULL;
	char *ip = NULL;
	short port = 0;
	int c;

	int fd, sock;
	char head[HEAD_LEN];
	struct stat sb;

	if (memcmp(argv[1], "push", 4))
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

	fd = deflate(path);
	if (fd < 0)
		die_on_user_error("can not use path %s", path);
	if (fstat(fd, &sb) < 0)
		die_on_system_error("fstat");

	sock = connect_to(ip, port);
	if (snprintf(head, HEAD_LEN, HEAD_FMT, VERSION, (uintmax_t)sb.st_size) < 0)
		die_on_user_error("snprintf");

	write_or_die(sock, head, strlen(head));

	send_fd_or_die(sock, fd, sb.st_size);

	close(sock);
	printf("Successfully send %s\n", path);
	exit(0);
}
