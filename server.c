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
	char *path;
	struct message *msg;
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = &do_nothing;

	msg = parse_request_head(sock);
	if (msg == NULL)
		fatal("[%llu] corruppt header", (uintmax_t)getpid());

	if (msg->action == PUSH){
		path = handle_push(sock, msg->length);

		printf("[%llu] inflating\n", (uintmax_t)getpid());
		if (inflate(path) < 0)
			fatal("can not inflate %s", path);
		printf("[%llu] successfully received %llu length file\n", (uintmax_t)getpid(), (uintmax_t)msg->length);
		fflush(stdout);
		exit(0);
	} else if (msg->action == GET){
		exit(handle_get(sock, msg->length));
	}
}

static int run(const char *ip, short port, unsigned int pending){
	int sock, cltsock;
	struct sockaddr cltaddr;
	char cltip[BUFSIZ];
	socklen_t cltaddrlen;

	sock = bind_to(ip, port);
	if (sock < 0)
		fatal("can not bind to %s:%d", ip, port);
	printf("bind to %s\n", ip);

	if (listen(sock, pending))
		fatal("listen error");

	for (;;){
		cltsock = accept(sock, &cltaddr, &cltaddrlen);
		if (cltsock < 0)
			fatal("accept error");
		if (inet_ntop(AF_INET, &cltaddr, cltip, cltaddrlen) == NULL)
			error("inet_ntop error when geting client's address");
		printf("accept from %s\n", cltip);
		fflush(stdout);
		switch (fork()){
			case -1:
				fatal("fork");
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
					fatal("%s is not a valid port", optarg);
				break;
			case '?':
				break;
			case ':':
				fatal("missing argument");
				break;
			default:
				fatal("getopt returned character code 0%o\n", c);
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
				fatal("could not use path as recv dir");
		} else
			fatal("could stat %s", path);
	}
	else
		if (!S_ISDIR(sb.st_mode))
			fatal("%s is not directory", path);

	if (chdir(path))
		fatal("could not change dir to %s", path);

	exit(run(ip, port, 20));
}
