#include "compress.h"

/**
 * This use tar to deflate file, if the file size is 0, then use --create
 * option, use --append otherwise, return 0 on success, -1 on error.
 */
static int deflate_one(const char *tar_path, const char *path){
	char *path1 = xstrdup(path);
	struct stat sb;
	int cld;
	int status;

	switch (cld = fork()){
		case -1:
			return -1;
			break;
		case 0:
			/* child */
			if (stat(tar_path, &sb))
				return -1;
			/**
			 * I know this is very ugly, but I want to keep tar file only
			 * contain useful content, if we do not deflate one by one,
			 * deflate them at once like :
			 * 'tar cf t.tar.gz /foo/bar /bar/foo'
			 * we will get useless foo and bar dirs
			 * but this will be very slow, because we will call fork as
			 * many times as the number files we want to deflate.
			 */
			if (sb.st_size == 0)
				execlp("tar", "tar", "--create", "--file",
						tar_path, "--directory", dirname(path1), basename(path1), (char *)NULL);
			else
				execlp("tar", "tar", "--append", "--file",
						tar_path, "--directory", dirname(path1), basename(path1), (char *)NULL);
			return -1;
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				return -1;
			if (status != 0)
				return -1;
			break;
	}
	free(path1);
	return 0;
}

/**
 * path of deflate should be NULL terminated string array,
 * and it call deflate_one of each path one by one.
 * If all path could not be deflated it returns -1, return
 * fd otherwise.
 */
int deflate(char **path){
	char *template = xstrdup("/tmp/SYNC_deflate_XXXXXX");
	int fd;
	int status;
	int i;
	int numerror = 0;

	fd = create_tmp(template);
	if (fd < 0)
		return error("could not create temp file");

	for (i = 0; path[i]; i++){
		status = deflate_one(template, path[i]);
		if (status < 0){
			error("could not deflate %s", path[i]);
			numerror++;
		}
	}
	if (numerror == i)
		return -1;
	else
		return fd;
}

int inflate(const char *path){
	pid_t cld;
	int status;
	switch (cld = fork()){
		case -1:
			return -1;
			break;
		case 0:
			/* child */
			execlp("tar", "tar", "--extract", "--file", path, (char *)NULL);
			return -1;
			break;
		default:
			/* father */
			if (waitpid(cld, &status, 0) < 0)
				fatal("waitpid");
			if (status != 0)
				return -1;
			break;
	}
	return 0;
}
