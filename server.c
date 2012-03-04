#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <getopt.h>
#include "compress.h"
#include "transport.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

static char use[] = "sync-server [-h ip | --host ip] [-p port | --port port] <recv_path>";

/* return static allocated numeric IP address */
static char *gethostip(){
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	static char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1)
		die_on_system_error("getifaddrs");

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		family = ifa->ifa_addr->sa_family;

		if (!strcmp(ifa->ifa_name,"lo"))
			continue;
		/* we do not have plan to support ipv6 right now */
		if (family != AF_INET)
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
				host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0)
			die_on_user_error("getnameinfo() failed: %s\n", gai_strerror(s));
	}
	freeifaddrs(ifaddr);
	return host;
}

static void handle_client(int sock){
	off_t size;
	char *path;
	size = parse_head(sock);
	path = process_body(sock, size);
	printf("[%llu] inflating\n", (uintmax_t)getpid());
	if (inflate(path) < 0)
		die_on_user_error("can not inflate %s", path);
	printf("[%llu] successfully received %llu length file\n", (uintmax_t)getpid(), (uintmax_t)size);
	fflush(stdout);
	_exit(0);
}

int main(int argc, char *argv[]){
	char *path = NULL;
	short port = 0;
	char *ip = NULL;
	int c;

	struct stat sb;
	int sock, cltsock;
	struct sockaddr_in addr;
	struct sockaddr cltaddr;
	char cltip[BUFSIZ];
	socklen_t cltaddrlen;

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

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		die_on_system_error("socket");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;

	if (ip == NULL)
		die_on_user_error("can not find a suitable IP to bind, \
maybe you want to use -h=[IP] to specified manually");
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		die_on_user_error("%s is not a ip", ip);
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
		die_on_system_error("bind");
	printf("bind to %s\n", ip);
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
