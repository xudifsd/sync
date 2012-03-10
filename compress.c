#include "compress.h"

int deflate(const char *path){
	char *template = strdup("/tmp/SYNC_deflate_XXXXXX");
	char *path1 = strdup(path);
	int fd;
	int status;
	pid_t cld;

	fd = create_tmp(template);
	if (fd < 0)
		die_on_user_error("could not create temp file");

	switch (cld = fork()){
		case -1:
			die_on_system_error("fork");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--create", "--gzip", "--file", template, "--directory", dirname(path1), basename(path1), (char *)NULL);
			die_on_system_error("execlp");
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				die_on_system_error("waitpid");
			if (status != 0)
				die_on_user_error("child can not deflate %s", path);
			break;
	}
	free(path1);
	return fd;
}

int inflate(const char *path){
	pid_t cld;
	char *path1 = strdup(path);
	int status;
	switch (cld = fork()){
		case -1:
			die_on_system_error("fork");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--extract", "--gzip", "--file", path1, (char *)NULL);
			die_on_system_error("execlp");
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				die_on_system_error("waitpid");
			if (status != 0)
				die_on_user_error("child can not inflate %s", path1);
			break;
	}
	return 0;
}
