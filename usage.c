#include "usage.h"

void usage(const char *err){
	fprintf(stderr, "%s\n", err);
	exit(3);
}

void error(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);

	return;
}

void die_on_system_error(const char *err){
	perror(err);
	exit(1);
}

void die_on_user_error(const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);

	exit(2);
}
