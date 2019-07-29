CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -g -Wall -pedantic -L. -I. #-fsanitize=address
SOCKNAME = objstore.sock
VALGRIND_FLAGS = --leak-check=full #-v

all: clean dir server client

dir :
	mkdir data
	mkdir data/fdizzia

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

dclient: client
	valgrind $(VALGRIND_FLAGS) ./client

rclient: client
	@./client

rserver: server
	@./server

server: server.c thread_worker.c parser.c libplug.a
	$(CC) $(CFLAGS) $< thread_worker.c parser.c -o $@ -lplug -lpthread

client: client.c libplug.a
	$(CC) $(CFLAGS) $< -o $@ -lplug

libplug.a: lib.o
	ar rvs $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME) ./*.a
	$(RM) -r ./data/* ./data
	clear
.PHONY: rclient rserver clean dclient dserver all
