#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "usage.h"

extern char *gethostip();
extern int connect_to(const char *ip, short port);
extern int bind_to(const char *ip, short port);
#endif /* NETWORK_H */
