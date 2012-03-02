#ifndef USAGE_H
#define USAGE_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

extern void usage(const char *err);
extern void error(const char *fmt, ...);
extern void die_on_system_error(const char *err);
extern void die_on_user_error(const char *fmt, ...);

#endif /* USAGE_H */
