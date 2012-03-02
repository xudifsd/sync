#include "write_or_die.h"

int write_in_full(int fd, void *buf, size_t count){
	int written = 0;
	int nr;
	while (written < count){
		nr = xwrite(fd, buf + written, count + written);
		if (nr < 0)
			return -1;
		written += nr;
	}
	return 0;
}

void write_or_die(int fd, void *buf, size_t count){
	if (write_in_full(fd, buf, count) < 0)
		die_on_system_error("write_in_full");
}
