#ifndef WRITE_OR_DIE_H
#define WRITE_OR_DIE_H

#include <unistd.h>
#include "wrapper.h"
#include "usage.h"

extern void write_or_die(int fd, void *buf, size_t count);

#endif /* WRITE_OR_DIE_H */
