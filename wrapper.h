#ifndef WRAPPER_H
#define WRAPPER_H

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h> /* for read_with_timeout */
#include "usage.h"

#define TIMEOUT 60

extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern char *xstrdup(const char *str);

extern ssize_t xread(int fd, void *buf, size_t count);
extern ssize_t read_in_full(int fd, void *buf, size_t count);
extern ssize_t read_with_timeout(int fd, void *buf, size_t count, unsigned int sec);

extern ssize_t xwrite(int fd, const void *buf, size_t count);
extern ssize_t write_in_full(int fd, void *buf, size_t count);

#endif /* WRAPPER_H */
