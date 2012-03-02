#include "transport.h"

off_t parse_head(int fd){
	double version;
	off_t length;
	FILE *fp;
	char line[1024];
	char *p;

	/* parse version */
	fp = fdopen(fd, "r");
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
		die_on_user_error("version %f is not supported in current version %f", version, VERSION);

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
		goto CORRUPT;
	return length;
 CORRUPT:
	die_on_user_error("parse_head: corrupt header");
	return 0;
}

char *process_body(int fd, off_t expected_size){
	static char template[] = "/tmp/SYNC_XXXXXX";
	char buf[BUFSIZ];
	struct stat sb;
	ssize_t nr = 0;
	off_t received = 0;
	int filefd;

	filefd = mkstemp(template);

	/* FIXME share code with client.c: send_fd_or_die */
	while (received < expected_size){
		nr = xread(fd, buf, BUFSIZ);
		if (nr < 0)
			die_on_system_error("read from socket");
		if (nr == 0)
			break;
		write_or_die(filefd, buf, nr);
		received += nr;
	}
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
	die_on_user_error("received unexpected size");
	return NULL;
}
