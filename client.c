#include <arpa/inet.h>
#include <inttypes.h>
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

static char use[] = "sync-client push <path> <IP> [port]";

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
	char *path;
	char *ip;
	short port;
	int fd, sock;
	char head[HEAD_LEN];
	struct stat sb;
	if (argc == 2){
		if (!strcmp(argv[1], "--version")){
			printf("%f\n", VERSION);
			exit(3);
		} else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
			usage(use);
		else
			usage(use);
	} else if ((argc == 4 || argc == 5) && !strcmp("push", argv[1])){
		path = argv[2];
		ip = argv[3];
		port = (argc == 5) ? atoi(argv[4]) : SERVER_PORT;
		if (port == 0)
			die_on_user_error("unknow port %d", port);
	} else
		usage(use);

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
