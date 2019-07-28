#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>

#include <lib.h>

int fd = -1;

#define STORE_LENGTH 11
#define REGISTER_LENGTH 12


ssize_t getChunkSize(ssize_t N){
 ssize_t current_chunk = DEFAULT_CHUNK_SIZE;
 while(current_chunk < N)
	  current_chunk = current_chunk * 2;

 return current_chunk;
}

size_t getNumberOfDigits(int k){
	if(k == 0)return 1;
  if(k < 0){printf("BAKANA\n");return -1;}

  int len;
  for(len = 0; k > 0; len++)
    k = k/10;

	return len;
}

int os_store(char *name, void *block, size_t len) {
  ssize_t N = STORE_LENGTH + strlen(name) + getNumberOfDigits(len) + len;
  ssize_t chunks = getChunkSize(N);

  char* buff = calloc(chunks, sizeof(char));
  sprintf(buff,"STORE %s %lu \n %s",name, len, (char*)block);

	printf("buff: %s | len: %ld | chunk: %ld\n",buff ,N ,chunks);

	writen(fd, buff, chunks);
	//writen(fd, block, len);

	return true;
}


int os_connect(char *name) {
	int len = strlen(name);/*
	int current_chunk = DEFAULT_CHUNK_SIZE;

  while(current_chunk < len + REGISTER_LENGTH)
	  current_chunk = current_chunk * 2;
*/
  ssize_t chunks = getChunkSize(len+REGISTER_LENGTH);

	printf("name: %s | strlen: %d | chunk: %ld\n",name ,len ,chunks);

	char *buff = calloc(chunks, sizeof(char));

  sprintf(buff,"REGISTER %s \n",name);

	writen(fd, buff, chunks);
  free(buff);

	return true;
}

int os_delete(char* name){
 //TODO

 return true;
}

bool str_equals(char* a, char* b){
 ASSERT_NULL(a,"a is NULL");
 ASSERT_NULL(b,"b is NULL");

 return (strcmp(a,b) == 0);
}

int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

  while(left > 0) {
	   if ((r = read((int)fd ,bufptr,left)) == -1) {
	     if (errno == EINTR)
        continue;

      return -1;
	   }

  if (r == 0)
   return 0;

  left -= r;
	bufptr += r;
  }

  return size;
}


int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

  while(left > 0) {
	 if((r = write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	 }

   if(r == 0)
    return 0;

   left -= r;
	 bufptr += r;
  }

    return 1;
}
