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
 * FIXME implement get method. The request of get should like this:
 * SYNC:0.2
 * GET
 * LENGTH:xxx
 * '\n'
 * path1
 * path2
 * ...
 */
int handle_get(int fd, size_t expected_size){
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
