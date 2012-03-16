#include "network.h"

/**
 * return static allocated numeric IP address, we first exclude lo
 * interface, but if we can not find a suitable addr, we return
 * "127.0.0.1". That means we will always return a usable addr.
 */
char *gethostip(){
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	static char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1)
		die_on_system_error("getifaddrs");

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		family = ifa->ifa_addr->sa_family;

		if (!strcmp(ifa->ifa_name,"lo"))
			continue;
		/* we do not have plan to support ipv6 right now */
		if (family != AF_INET)
			continue;

		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
				host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0)
			die_on_user_error("getnameinfo() failed: %s\n", gai_strerror(s));
	}
	freeifaddrs(ifaddr);
	if (!memcmp(host, "", 1))
		strcpy(host, "127.0.0.1");
	return host;
}

int connect_to(const char *ip, short port){
	int sock;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		die_on_user_error("unknow IP: %s\n", ip);
	addr.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		die_on_system_error("socket");
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die_on_system_error("connect");
	return sock;
}

int bind_to(const char *ip, short port){
	int sock;
	struct sockaddr_in addr;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return sock;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;

	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
		return -1;
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
		return -2;
	return sock;
}
