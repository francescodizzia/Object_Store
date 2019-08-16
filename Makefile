CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -g -Wall -pedantic -L. -I. #-fsanitize=address
SOCKNAME = objstore.sock
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all #-v

SERVER_COMPILE = server.c thread_worker.c parser.c hashtable.c

all: clean dir server client

dir :
	mkdir data

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

test: server client
	./test_base.sh
	./testsum.sh

server: $(SERVER_COMPILE) libplug.a
	$(CC) $(CFLAGS)  $(SERVER_COMPILE) -o $@ -lplug -lpthread

client: client.c libplug.a
	$(CC) $(CFLAGS) $< -o $@ -lplug -lpthread

libplug.a: lib.o
	ar rvs $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME) ./testout.log ./*.a
	$(RM) -r ./data/* ./data
	clear
.PHONY: rclient rserver clean dclient dserver all
