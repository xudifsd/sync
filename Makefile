CFLAGS = -Wall -g -c
OBJS = compress.o file.o network.o message.o usage.o wrapper.o write_or_die.o
LIBS = compress.h file.h network.h message.h usage.h wrapper.h write_or_die.h
CC = gcc

PREFIX = $(HOME)

all: sync-server sync-client

sync-client: $(OBJS) client.o
	$(CC) -o sync-client client.o $(OBJS)

sync-server: $(OBJS) server.o
	$(CC) -o sync-server server.o $(OBJS)


client.o: $(LIBS) client.c
	$(CC) $(CFLAGS) client.c

file.o: $(LIBS) file.c
	$(CC) $(CFLAGS) file.c

server.o: $(LIBS) server.c
	$(CC) $(CFLAGS) server.c

compress.o: compress.c $(LIBS)
	$(CC) $(CFLAGS) compress.c

network.o: $(LIBS) network.c
	$(CC) $(CFLAGS) network.c

message.o: message.c $(LIBS)
	$(CC) $(CFLAGS) message.c

usage.o: usage.c $(LIBS)
	$(CC) $(CFLAGS) usage.c

wrapper.o: wrapper.c $(LIBS)
	$(CC) $(CFLAGS) wrapper.c

write_or_die.o: write_or_die.c $(LIBS)
	$(CC) $(CFLAGS) write_or_die.c

clean:
	-rm $(OBJS) sync-server sync-client client.o server.o

install: all
	cp sync-server sync-client $(PREFIX)/bin
	
uninstall:
	-rm $(PREFIX)/bin/sync-server $(PREFIX)/bin/sync-client
