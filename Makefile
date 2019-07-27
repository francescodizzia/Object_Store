CC = gcc
CFLAGS = -std=c99 -g -Wall -pedantic -L. -I. -fsanitize=address
SOCKNAME = objstore.sock
VALGRIND_FLAGS = --leak-check=full #-v

all: clean server client

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

dclient: client
	valgrind $(VALGRIND_FLAGS) ./client

rclient: client
	@./client

rserver: server
	@./server

server: server.c libplug.a
	$(CC) $(CFLAGS) $< -o $@ -lplug -lpthread

client: client.c libplug.a
	$(CC) $(CFLAGS) $< -o $@ -lplug

libplug.a: lib.o
	ar rvs $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME) ./*.a
	clear
.PHONY: rclient rserver clean dclient dserver all
