#include "message.h"

/* FIXME use timeout to parse head */
off_t parse_request_head(int fd){
	double version;
	off_t length;
	FILE *fp;
	char line[1024];
	char *p;

	/* parse version */
	fp = fdopen(fd, "r");
	setbuf(fp, NULL);
	if (fp == NULL){
		die_on_system_error("fdopen");
	}
	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (memcmp("SYNC", line, 4))
		goto CORRUPT;
	p = strchr(line, ':');
	if (p == NULL)
		goto CORRUPT;
	p++;
	version = atof(p);
	if (version == 0)
		goto CORRUPT;
	if (version > VERSION)
		die_on_user_error("version %.2f is not supported in current version %.2f", version, VERSION);

	/* parse length*/
	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (memcmp("LENGTH", line, 6))
		goto CORRUPT;
	p = strchr(line, ':');
	if (p == NULL)
		goto CORRUPT;
	p++;
	length = atoll(p);
	if (length == 0)
		goto CORRUPT;

	if (fgets(line, 1024, fp) == NULL)
		goto CORRUPT;
	if (strlen(line) != 1 || memcmp(line, "\n", 1))
		if (strlen(line) != 2 || memcmp(line, "\r\n", 2))
			goto CORRUPT;
	return length;
 CORRUPT:
	die_on_user_error("parse_request_head: corrupt header");
	return 0;
}

char *process_request_body(int fd, off_t expected_size){
	char *template = strdup("/tmp/SYNC_REC_XXXXXX");
	struct stat sb;
	off_t received = 0;
	int filefd;

	filefd = create_tmp(template);
	if (fd < 0)
		die_on_user_error("could not create temp file");

	received = copy_between_fd(fd, filefd, -1, expected_size, 0);

	if (received != expected_size)
		goto SIZE_ERROR;
	close(fd);
	close(filefd);

	if (stat(template, &sb) < 0)
		die_on_system_error("stat");
	if (sb.st_size != expected_size)
		goto SIZE_ERROR;
	return template;
 SIZE_ERROR:
	die_on_user_error("[%llu] received unexpected size", (uintmax_t)getpid());
	return NULL;
}

/**
 * This return the number byte that actually written to request,and caller
 * should ensure the request have enought space, they can use HEAD_LEN for
 * that.
 * Currently, we can only use PUSH in action, GET is not supported yet.
 */
int generate_request_header(int action, off_t length, char *request, size_t count){
	return snprintf(request, count, HEAD_FMT, VERSION, (uintmax_t)length);
}
