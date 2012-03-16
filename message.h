#ifndef MESSAGE_H
#define MESSAGE_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "file.h"
#include "usage.h"
#include "wrapper.h"
#include "write_or_die.h"

#define SERVER_PORT 8081
#define VERSION 0.2

#define PUSH 1
#define GET 2

#define HEAD_FMT "SYNC:%f\n%s\nLENGTH:%llu\n\n"
#define HEAD_LEN sizeof(HEAD_FMT)+13	/* enough to hold expanded head */

struct message{
	double version;
	int action;
	off_t length;
};

extern struct message *parse_request_head(int fd);	/* parse head, return expected static allocated structure */
extern char *handle_push(int fd, off_t expected_size);	/* return saved path */
extern int handle_get(int fd, size_t expected_size);
extern int generate_request_header(int action, off_t length, char *request, size_t count);

#endif /* MESSAGE_H */
