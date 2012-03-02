#include "wrapper.h"

void *xmalloc(size_t size){
	void *p;
	p = malloc(size);
	if (p == NULL && size != 0)
		die_on_user_error("out of memory when invoking malloc(%d)", size);
	return p;
}

void *xrealloc(void *ptr, size_t size){
	void *p;
	p = realloc(ptr, size);
	if (p == NULL && size != 0)
		die_on_user_error("out of memory when invoking realloc(%d)", size);
	return p;
}

/**
 * xread auto restart when encounter a restartable error.
 */
ssize_t xread(int fd, void *buf, size_t count){
	size_t nr = 0;
	for (;;){
		nr = read(fd, buf, count);
		if (nr < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		return nr;
	}
}

/**
 * xwrite auto restart when encounter a restartable error.
 */
ssize_t xwrite(int fd, const void *buf, size_t count){
	size_t nr = 0;
	for (;;){
		nr = write(fd, buf, count);
		if (nr < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		return nr;
	}
}
