#include "message.h"
//#define DEBUG 1

/**
 * return static allocated structure, return NULL on error.
 * FIXME use timeout to parse head.
 */
struct message *parse_request_head(int fd){
	static struct message msg;
	FILE *fp;
	char line[1024];
	char *p;

	memset(&msg, 0, sizeof(msg));

	/* parse version */
	fp = fdopen(fd, "r");
	setbuf(fp, NULL);
	if (fp == NULL)
		return NULL;

	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (memcmp("SYNC", line, 4))
		goto CORRUPT;
	p = strchr(line, ':');
	if (p == NULL)
		goto CORRUPT;
	p++;
	msg.version = atof(p);
	if (msg.version == 0)
		goto CORRUPT;
	if (msg.version > VERSION){
		error("version %.2f is not supported in current version %.2f", msg.version, VERSION);
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "version is %.2f\n", msg.version);
#endif

	/* parse action */
	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
#ifdef DEBUG
	fprintf(stderr, "line is %s\n", line);
#endif
	if (!strcmp("PUSH\n", line) || !strcmp("PUSH\r\n", line))
		msg.action = PUSH;
	else if (!strcmp("GET\n", line) || !strcmp("GET\r\n", line))
		msg.action = GET;
	else
		goto CORRUPT;
#ifdef DEBUG
	fprintf(stderr, "action is %d\n", msg.action);
#endif

	/* parse length */
	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (memcmp("LENGTH", line, 6))
		goto CORRUPT;
	p = strchr(line, ':');
	if (p == NULL)
		goto CORRUPT;
	p++;
	msg.length = atoll(p);
	if (msg.length == 0)
		goto CORRUPT;

	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (strlen(line) != 1 || memcmp(line, "\n", 1))
		if (strlen(line) != 2 || memcmp(line, "\r\n", 2))
			goto CORRUPT;
#ifdef DEBUG
	fprintf(stderr, "length is %lld\n", (uintmax_t)msg.length);
#endif
	return &msg;
 CORRUPT:
	close(fd);
	return NULL;
}

void make_push(int sock, int fd){
	struct stat sb;
	char head[HEAD_LEN];

	if (fstat(fd, &sb) < 0)
		fatal("fstat failed");
	if (generate_request_header(PUSH, (uintmax_t)sb.st_size, head, HEAD_LEN) == 0)
		fatal("can not generate header");

	write_or_die(sock, head, strlen(head));

	copy_between_fd(fd, sock, STDOUT_FILENO, sb.st_size, 1);

	close(sock);
	printf("Successfully send %lld bytes\n", (uintmax_t)sb.st_size);
	exit(0);
}

void make_get(int sock, char *path[]){
	char head[HEAD_LEN];
	struct message *msg;
	char *recpath;
	size_t size = 0;
	struct stat sb;
	int i;

	for (i = 0; path[i]; i++){
#ifdef DEBUG
	fprintf(stderr, "request %s\n", path[i]);
#endif
		size += strlen(path[i]) + 1; /* 1 is for '\n' for each path */
	}

	if (generate_request_header(GET, size, head, HEAD_LEN) == 0)
		fatal("can not generate header");

	write_or_die(sock, head, strlen(head));

	for (i = 0; path[i]; i++){
		write_or_die(sock, (void *)path[i], strlen(path[i]));
		write_or_die(sock, "\n", 1);
	}

	/* we should shutdown write end so that the server can see EOF */
	if (shutdown(sock, SHUT_WR))
		fatal("could not shutdown write end of socket");

#ifdef DEBUG
	fprintf(stderr, "about to parse server response\n");
#endif
	/* we expect server return PUSH header, we die if not */
	msg = parse_request_head(sock);
	if (msg == NULL)
		fatal("server did not respond correctly");
	if (msg->action != PUSH)
		fatal("server did not respond correctly");

	recpath = handle_push(sock, msg->length);
	if (recpath == NULL)
		fatal("error occured when receiving file of %lld length", (uintmax_t)msg->length);

#ifdef DEBUG
	fprintf(stderr, "received %s\n", recpath);
#endif
	memset(&sb, 0, sizeof(sb));
	if (stat(recpath, &sb))
		fatal("could not stat %s", recpath);

	if (sb.st_size == msg->length)
		printf("Successfully received %lld bytes\n", (uintmax_t)sb.st_size);
	else
		fatal("received file of %lld length, but expect %lld\n", (uintmax_t)sb.st_size, (uintmax_t)msg->length);

	printf("inflating %s\n", recpath);
	if (inflate(recpath) < 0)
		fatal("inflate error");

	close(sock);
	exit(0);
}

char *handle_push(int fd, off_t expected_size){
	char *template = strdup("/tmp/SYNC_REC_XXXXXX");
	struct stat sb;
	off_t received = 0;
	int filefd;

	filefd = create_tmp(template);
	if (fd < 0)
		fatal("could not create temp file");

	received = copy_between_fd(fd, filefd, -1, expected_size, 0);

	if (received != expected_size)
		goto SIZE_ERROR;
	close(fd);
	close(filefd);

	if (stat(template, &sb) < 0)
		fatal("could not stat template file %s", template);
	if (sb.st_size != expected_size)
		goto SIZE_ERROR;
	return template;
 SIZE_ERROR:
	fatal("[%llu] received unexpected size", (uintmax_t)getpid());
	return NULL;
}

/**
 * We respond GET action with PUSH
 */
int handle_get(int sock, size_t expected_size){
	FILE *fp;
	char line[1024];

	int fd;
	int nr = 0;
	int alloc = 5;
	char **paths = xmalloc(alloc * sizeof(char *));
	struct stat sb;

	/* parse requested path, if we don't have, just ignore */
	fp = fdopen(sock, "r");
	setbuf(fp, NULL);
	if (fp == NULL)
		return -1;

	for (;;){
		if (fgets(line, 1024, fp) == NULL){
			if (feof(fp))
				break;
			fatal("corrupt GET body");
		}
		/* replace '\n' with '\0' */
		line[strlen(line) - 1] = '\0';

#ifdef DEBUG
		fprintf(stderr, "client request path '%s', before stat(line, &sb)\n", line);
#endif
		if (stat(line, &sb))
			continue; /* ignore missing */

		if (nr >= alloc - 1){ /* the last one for NULL */
			paths = xrealloc(paths, (alloc*3/2) * sizeof(char *));
			alloc = alloc*3/2;
		}

#ifdef DEBUG
		fprintf(stderr, "add path %s\n", line);
#endif

		paths[nr] = xstrdup(line);
		nr++;
	}
	paths[nr] = NULL;

	fd = deflate(paths);
	if (fd < 0)
		fatal("failed to deflate");

	make_push(sock, fd);
	return 0;
}

/**
 * This return the number byte that actually written to request,and caller
 * should ensure the request have enought space, they can use HEAD_LEN for
 * that.
 * Currently, we can only use PUSH in action, GET is not supported yet.
 */
int generate_request_header(int action, off_t length, char *request, size_t count){
	int nr;
	if (action == PUSH)
		nr = snprintf(request, count, HEAD_FMT, VERSION, "PUSH", (uintmax_t)length);
	else if (action == GET)
		nr = snprintf(request, count, HEAD_FMT, VERSION, "GET", (uintmax_t)length);
	else
		nr = 0;
	return nr;
}
