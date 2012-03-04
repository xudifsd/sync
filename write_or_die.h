#ifndef WRITE_OR_DIE_H
#define WRITE_OR_DIE_H

#include <unistd.h>
#include "wrapper.h"
#include "usage.h"

extern size_t copy_between_fd(int from, int to, int report_fd, off_t expected_size, int die_on_error);
extern void write_or_die(int fd, void *buf, size_t count);

#endif /* WRITE_OR_DIE_H */
