CFLAGS = -Wall -g -c
OBJS = compress.o transport.o usage.o wrapper.o write_or_die.o
LIBS = compress.h transport.h usage.h wrapper.h write_or_die.h
CC = gcc


all: sync-server sync-client

sync-client: $(OBJS)
	$(CC) -o sync-client client.c $(OBJS)

sync-server: $(OBJS)
	$(CC) -o sync-server server.c $(OBJS)


compress.o: compress.c $(LIBS)
	$(CC) $(CFLAGS) compress.c

transport.o: transport.c $(LIBS)
	$(CC) $(CFLAGS) transport.c

usage.o: usage.c $(LIBS)
	$(CC) $(CFLAGS) usage.c

wrapper.o: wrapper.c $(LIBS)
	$(CC) $(CFLAGS) wrapper.c

write_or_die.o: write_or_die.c $(LIBS)
	$(CC) $(CFLAGS) write_or_die.c

clean:
	-rm $(OBJS) sync-server sync-client
