#include "compress.h"

int deflate(const char *path){
	char *template = strdup("/tmp/SYNC_deflate_XXXXXX");
	char *path1 = strdup(path);
	int fd;
	int status;
	pid_t cld;

	fd = create_tmp(template);
	if (fd < 0)
		return error("could not create temp file");

	switch (cld = fork()){
		case -1:
			fatal("fork error");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--create", "--gzip", "--file", template, "--directory", dirname(path1), basename(path1), (char *)NULL);
			fatal("could not execlp, maybe you did not installed tar");
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				fatal("waitpid error");
			if (status != 0)
				return error("child can not deflate %s", path);
			break;
	}
	free(path1);
	return fd;
}

int inflate(const char *path){
	pid_t cld;
	int status;
	switch (cld = fork()){
		case -1:
			fatal("fork error");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--extract", "--gzip", "--file", path, (char *)NULL);
			fatal("could not execlp, maybe you did not installed tar");
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				fatal("waitpid");
			if (status != 0)
				return error("child can not inflate %s", path);
			break;
	}
	return 0;
}
