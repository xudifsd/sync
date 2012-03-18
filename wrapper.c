#include "wrapper.h"

void *xmalloc(size_t size){
	void *p;
	p = malloc(size);
	if (p == NULL && size != 0)
		fatal("out of memory when invoking malloc(%d)", size);
	return p;
}

void *xrealloc(void *ptr, size_t size){
	void *p;
	p = realloc(ptr, size);
	if (p == NULL && size != 0)
		fatal("out of memory when invoking realloc(%d)", size);
	return p;
}

char *xstrdup(const char *str){
	char *p;
	p = strdup(str);
	if (p == NULL && str != NULL)
		fatal("out of memory when invoke strdup");
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

ssize_t read_in_full(int fd, void *buf, size_t count){
	char *p = buf;
	ssize_t total = 0;

	while (count > 0) {
		ssize_t loaded = xread(fd, p, count);
		if (loaded < 0)
			return -1;
		if (loaded == 0)
			return total;
		count -= loaded;
		p += loaded;
		total += loaded;
	}

	return total;
}

static void alarm_handler(int sig){
	/**
	  * This is for read_with_timeout. We do nothing, we just use alarm
	  * to interrupt blocked reading
	 */
}

/**
 * read_with_timeout is same as read_in_full, but use timeout to prevent
 * waste time in reading, this is especially useful in network transport.
 */
ssize_t read_with_timeout(int fd, void *buf, size_t count, unsigned int sec){
	ssize_t nr;
	struct sigaction sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = &alarm_handler;
	if (sigaction(SIGALRM, &sa, NULL) == -1)
		error("sighandler can not be established");
	alarm(sec);
	nr = read_in_full(fd, buf, count);
	alarm(0); /* cancel alarm */
	return nr;
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

ssize_t write_in_full(int fd, void *buf, size_t count){
	const char *p = buf;
	ssize_t total = 0;

	while (count > 0){
		ssize_t written = xwrite(fd, p, count);
		if (written < 0)
			return -1;
		if (!written) {
			errno = ENOSPC;
			return -1;
		}
		count -= written;
		p += written;
		total += written;
	}

	return total;
}
