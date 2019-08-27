CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -g -Wall -pedantic -L. -I. #-fsanitize=address
SOCKNAME = objstore.sock
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all #-v

default_target: all

all: clean dir server client

dir:
	mkdir data

test: server client
	./test_base.sh
	./testsum.sh

test2:
	./test_files.sh

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

server: server.c thread_worker.c parser.c hashtable.c common.c
	$(CC) $(CFLAGS) $? -o $@ -lpthread

client: client.c libobjstore.a
	$(CC) $(CFLAGS) $< -o $@ -lobjstore

libobjstore.a: lib.o common.o
	ar rvs $@ $?

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME) ./testout.log ./libobjstore.a ./testout.log
	$(RM) -r ./data/* ./data
	
.PHONY: clean dserver all
