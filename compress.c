#include "compress.h"

int deflate(const char *path){
	char template[] = "/tmp/SYNC_deflate_XXXXXX";
	int fd;
	int status;
	pid_t cld;
	fd = mkostemp(template, O_CLOEXEC);
	if (fd < 0)
		die_on_system_error("mkostemp");

	switch (cld = fork()){
		case -1:
			die_on_system_error("fork");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--create", "--file", template, path, (char *)NULL);
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
/*	if (lseek(fd, 0, SEEK_SET))
		die_on_system_error("lseek");
		*/
	return fd;
}

int inflate(const char *path){
	pid_t cld;
	int status;
	switch (cld = fork()){
		case -1:
			die_on_system_error("fork");
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--extract", "--file", path, (char *)NULL);
			die_on_system_error("execlp");
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				die_on_system_error("waitpid");
			if (status != 0)
				die_on_user_error("child can not inflate %s", path);
			break;
	}
	return 0;
}
