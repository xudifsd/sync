#include "file.h"

static char *temp_file;

static void unlink_tmp(void){
	unlink(temp_file);
}

/**
 * This function create a temp file, and it unlink it after process
 * terminate. The argument should be allocated using heap.
 * FIXME: we should also prevent signal related terminate.
 */
int create_tmp(char *template){
	int fd;

	temp_file = template;
	fd = mkstemp(template);

	if (fd < 0 || fcntl(fd, F_SETFD, FD_CLOEXEC))
		return -1;

	atexit(&unlink_tmp);
	return fd;
}
