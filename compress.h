#ifndef COMPRESS_H
#define COMPRESS_H

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "usage.h"

extern int deflate(const char *path);
extern int inflate(const char *path);	/* inflate file specified by path */

#endif /* COMPRESS_H */
