#include "write_or_die.h"

void write_or_die(int fd, void *buf, size_t count){
	if (write_in_full(fd, buf, count) < 0){
		if (errno == EPIPE)
			exit(0);
		die_on_system_error("write error");
	}
}
