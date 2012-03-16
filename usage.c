#include "usage.h"

void usage(const char *err){
	fprintf(stderr, "%s\n", err);
	exit(3);
}

int error(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);

	return -1;
}

void fatal(const char *fmt, ...){
	fputs("fatal: ", stderr);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);

	exit(2);
}
