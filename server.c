#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "compress.h"
#include "network.h"
#include "message.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

static char use[] = "sync-server [-h ip | --host ip] [-p port | --port port] <recv_path>";

/* FIXME: I should go to some file like signalchain.c */
static void do_nothing(int sig){
	/* I was used to prevent SIGPIPE */
	return;
}

static void handle_client(int sock){
	off_t size;
	char *path;
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = &do_nothing;

	size = parse_request_head(sock);
	path = process_request_body(sock, size);
	printf("[%llu] inflating\n", (uintmax_t)getpid());
	if (inflate(path) < 0)
		die_on_user_error("can not inflate %s", path);
	printf("[%llu] successfully received %llu length file\n", (uintmax_t)getpid(), (uintmax_t)size);
	fflush(stdout);
	exit(0);
}

static int run(const char *ip, short port, unsigned int pending){
	int sock, cltsock;
	struct sockaddr cltaddr;
	char cltip[BUFSIZ];
	socklen_t cltaddrlen;

	sock = bind_to(ip, port);
	if (sock < 0)
		die_on_user_error("can not bind to %s:%d", ip, port);
	printf("bind to %s\n", ip);

	if (listen(sock, pending))
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
	return 0;
}

int main(int argc, char *argv[]){
	char *path = NULL;
	short port = 0;
	char *ip = NULL;
	int c;

	struct stat sb;

	for (;;){
		static struct option long_options[] = {
			{"host", required_argument, NULL, 'h'},
			{"port", required_argument, NULL, 'p'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, ":h:p:",
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
				die_on_user_error("missing argument");
				break;
			default:
				die_on_user_error("getopt returned character code 0%o\n", c);
		}
	}
	if (optind < argc)
		path = argv[optind];
	if (path == NULL)
		usage(use);
	if (ip == NULL)
		ip = gethostip();
	if (port == 0)
		port = SERVER_PORT;

	/**
	 * child and parent both will use stdout, this will prevent some
	 * unexpected output
	 */
	setbuf(stdout, NULL);
	if (stat(path, &sb)){
		if (errno == ENOENT){
			if (mkdir(path, 0766))
				die_on_system_error("mkdir");
		} else
			die_on_system_error("stat");
	}
	else
		if (!S_ISDIR(sb.st_mode))
			die_on_user_error("%s is not directory", path);

	if (chdir(path))
		die_on_system_error("chdir");

	exit(run(ip, port, 20));
}
