#ifndef COMPRESS_H
#define COMPRESS_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "file.h"
#include "wrapper.h"
#include "usage.h"

extern int deflate(char **path);
extern int inflate(const char *path);	/* inflate file specified by path */

#endif /* COMPRESS_H */
