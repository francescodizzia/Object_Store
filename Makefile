CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -g -Wall -pedantic -L. -I. #-fsanitize=address
SOCKNAME = objstore.sock
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all #-v

SERVER_COMPILE = server.c thread_worker.c parser.c hashtable.c

all: clean dir server client

dir :
	mkdir data

test: server client
	./test_base.sh
	./testsum.sh

test2:
	./test_files.sh

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

server: $(SERVER_COMPILE) libobjstore.a
	$(CC) $(CFLAGS)  $(SERVER_COMPILE) -o $@ -lobjstore -lpthread

client: client.c libobjstore.a
	$(CC) $(CFLAGS) $< -o $@ -lobjstore -lpthread

evilClient: evilClient.c libobjstore.a
	$(CC) $(CFLAGS) $< -o $@ -lobjstore -lpthread

libobjstore.a: lib.o
	ar rvs $@ $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME) ./testout.log ./*.a
	$(RM) -r ./data/* ./data
	clear
.PHONY: clean dclient dserver all
