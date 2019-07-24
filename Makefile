CFLAGS = -std=c99 -g -Wall -pedantic
SOCKNAME = fradiz
VALGRIND_FLAGS = --leak-check=full
LIBS = -lpthread

dserver: server
	valgrind $(VALGRIND_FLAGS) ./server

dclient: client
	valgrind $(VALGRIND_FLAGS) ./client

rclient: client
	@./client

rserver: server
	@./server

client: client.o
	$(CC) client.o -o client $(LIBS)

server: server.o
	$(CC) server.o -o server $(LIBS)

clean:
	$(RM) ./*.o ./*.out ./server ./client ./$(SOCKNAME)
	clear
.PHONY: client server rclient rserver clean dclient
