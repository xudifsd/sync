#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "compress.h"
#include "transport.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

static char use[] = "sync-server <recv_path> [port]";

static void handle_client(int sock){
	off_t size;
	char *path;
	size = parse_head(sock);
	path = process_body(sock, size);
	if (inflate(path) < 0)
		die_on_user_error("can not inflate %s", path);
	printf("[%llu] successfully received %llu length file\n", (uintmax_t)getpid(), (uintmax_t)size);
	fflush(stdout);
	_exit(0);
}

int main(int argc, char *argv[]){
	char *path;
	short port;
	struct stat sb;
	int sock, cltsock;
	struct sockaddr_in addr;
	struct sockaddr cltaddr;
	char cltip[BUFSIZ];
	char ip[BUFSIZ];
	socklen_t cltaddrlen;
	if (argc == 2){
		if (!strcmp(argv[1], "--version")){
			printf("%f\n", VERSION);
			exit(3);
		} else if (!strcmp(argv[1], "--help")|| !strcmp(argv[1], "-h"))
			usage(use);
		else
			die_on_user_error("unknow option");
	} else if (argc == 2 || argc == 3){
		path = argv[1];
		port = (argc == 3) ? atoi(argv[2]) : SERVER_PORT;
	}

	if (stat(path, &sb)){
		if (errno == ENOENT){
			if (mkdir(path, 0766))
				die_on_system_error("mkdir");
		} else
			die_on_system_error("stat");
	}
	if (!S_ISDIR(sb.st_mode))
		die_on_user_error("%s is not directory", path);

	if (chdir(path))
		die_on_system_error("chdir");

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		die_on_system_error("socket");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	/* FIXME how to get ip? */
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		die_on_user_error("%s is not a ip", ip);
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
		die_on_system_error("bind");
	if (listen(sock, 10))
		die_on_system_error("listen");

	for (;;){
		cltsock = accept(sock, &cltaddr, &cltaddrlen);
		if (cltsock < 0)
			die_on_system_error("accept");
		if (inet_ntop(AF_INET, &cltaddr, cltip, cltaddrlen) == NULL)
			die_on_system_error("inet_ntop");
		printf("accept from %s\n", cltip);
		fflush(stdout);
		switch (fork()){
			case -1:
				die_on_system_error("fork");
				break;
			case 0:
				/* child */
				handle_client(cltsock);
				break;
			default:
				/* father */
				close(cltsock);
				continue;
		}
	}
}
