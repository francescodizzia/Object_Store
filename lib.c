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
#include <sys/stat.h>

#include <lib.h>

int fd = -1;

#define STORE_LENGTH 10
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


char* getUserPath(char* username){
  char* path = calloc(MAX_PATH_SIZE,sizeof(char));
  sprintf(path,"%s%s/",DATA_DIRECTORY,username);

  return path;
}

bool createFile(char* filename, void* data, char* username, size_t size){
 char path[MAX_PATH_SIZE];
 memset(path,'\0',MAX_PATH_SIZE);
 strcpy(path,DATA_DIRECTORY);

 strcat(path,username);
 strcat(path,"/");
 printf("path: %s\n",path);
 strcat(path,filename);

 int create_f = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);


 if(create_f == -1)
  return false;

 int w = writen(create_f, data, size);

 if(w == -1)
  printf("INCREDIBBBILE\n");

 close(create_f);
 return true;
}

int os_store(char *name, void *block, size_t len) {
  size_t store_size = STORE_LENGTH + strlen(name) + getNumberOfDigits(len);
  char* buff = calloc(store_size, sizeof(char));

  sprintf(buff,"STORE %s %lu \n ",name, len);
  //printf("%d vs %d",store_size,strlen(buff));
  printf("\n::%lu\n",len);

  writen(fd,buff,store_size);
  writen(fd,block,len);
  free(buff);

	return true;
}


int os_connect(char *name) {
  int len = strlen(name);
  int N = len+REGISTER_LENGTH;

  //if NOT REGISTERED
  //mkdir(getUserPath(name));

  char* buff = calloc(N, sizeof(char));
  sprintf(buff,"REGISTER %s \n",name);

	writen(fd, buff, N);
	//writen(fd, block, len);
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
