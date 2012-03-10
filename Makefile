CFLAGS = -Wall -g -c
OBJS = compress.o file.o transport.o usage.o wrapper.o write_or_die.o
LIBS = compress.h file.h transport.h usage.h wrapper.h write_or_die.h
CC = gcc


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

transport.o: transport.c $(LIBS)
	$(CC) $(CFLAGS) transport.c

usage.o: usage.c $(LIBS)
	$(CC) $(CFLAGS) usage.c

wrapper.o: wrapper.c $(LIBS)
	$(CC) $(CFLAGS) wrapper.c

write_or_die.o: write_or_die.c $(LIBS)
	$(CC) $(CFLAGS) write_or_die.c

clean:
	-rm $(OBJS) sync-server sync-client client.o server.o
