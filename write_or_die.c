#include "write_or_die.h"

/**
 * copy_between_fd is laid on top of write_in_full and read_with_timeout,
 * it returns total number byte that written, returned value is not equal
 * to expected_size to indicate an error. And it uses report_fd to report
 * progress, if invoke with report_fd == -1 then it report nothing.
 */
size_t copy_between_fd(int from, int to, int report_fd, off_t expected_size, int die_on_error){
	char buf[BUFSIZ];
	off_t total = 0;
	ssize_t nr;
	FILE *fp = NULL;

	if (report_fd != -1){
		fp = fdopen(report_fd, "w");
		if (fp == NULL)
			error("report_fd can be open with fdopen");
		setbuf(fp, NULL);
	}

	for (;;){
		nr = read_with_timeout(from, buf, BUFSIZ, TIMEOUT);
		if (nr == 0)
			return total;
		if (nr < 0){
			if (die_on_error)
				fatal("timeout when reading");
			else
				return total;
		}

		if (write_in_full(to, buf, nr) < 0){
			if (die_on_error)
				fatal("write error");
			else
				return total;
		}
		total += nr;

		/* FIXME we should not report that frequently */
		if (fp != NULL)
			fprintf(fp, "transported %.2f%%\n", (total/(double)expected_size)*100);
	}

	return total;
}

void write_or_die(int fd, void *buf, size_t count){
	if (write_in_full(fd, buf, count) < 0){
		if (errno == EPIPE)
			exit(0);
		fatal("write error");
	}
}
