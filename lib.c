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

#include<lib.h>

int fd = -1;

pthread_mutex_t printThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t awaken;

int os_store(char *name, void *block, size_t len) {
	char *buff = calloc(BUFFSIZE, sizeof(char));

  sprintf(buff,"STORE %s %lu \n ",name, len);

	writen(fd, buff, BUFFSIZE);
	writen(fd, block, len);
  free(buff);

	return true;
}
/*
int os_connect(char *name) {
	char *buff = calloc(BUFFSIZE, sizeof(char));

  sprintf(buff,"REGISTER %s \n ",name);

	writen(fd, buff, BUFFSIZE);
  free(buff);

	return true;
}
*/
//13+len
int os_connect(char *name) {
	int len = strlen(name);
	int current_chunk = DEFAULT_CHUNK_SIZE;

  while(current_chunk < len+12)
	  current_chunk = current_chunk * 2;

		printf("name: %s | strlen: %d | chunk: %d\n",name ,len ,current_chunk);

	char *buff = calloc(current_chunk, sizeof(char));

  sprintf(buff,"REGISTER %s \n",name);

	writen(fd, buff, current_chunk);
  free(buff);

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
