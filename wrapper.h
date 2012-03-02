#ifndef WRAPPER_H
#define WRAPPER_H

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "usage.h"

extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern ssize_t xread(int fd, void *buf, size_t count);
extern ssize_t read_in_full(int fd, void *buf, size_t count);
extern ssize_t xwrite(int fd, const void *buf, size_t count);
extern ssize_t write_in_full(int fd, void *buf, size_t count);

#endif /* WRAPPER_H */
