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
#include <stdbool.h>

#include <lib.h>
#include <hashtable.h>

int fd = -1;


#define REGISTER_LENGTH 12 //include \0, gli altri no
#define STORE_LENGTH 10
#define RETRIEVE_LENGTH 11
#define DELETE_LENGTH 9
#define DATA_MSG_LENGTH 8

char last_error_msg[256];

size_t getNumberOfDigits(size_t k){
  int len;
	if(k == 0)return 1;
  if(k < 0)return -1;

  for(len = 0; k > 0; len++)
    k = k/10;

	return len;
}


char* getUserPath(char* username){
  char* path = calloc(MAX_PATH_SIZE, sizeof(char));
  sprintf(path,"%s%s/",DATA_DIRECTORY, username);

  return path;
}

bool createFile(char* filename, void* data, char* username, size_t size){
 char path[MAX_PATH_SIZE];
 memset(path,'\0',MAX_PATH_SIZE);
 strcpy(path,DATA_DIRECTORY);

 strcat(path,username);
 strcat(path,"/");
 strcat(path,filename);

 int new_fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

 if(new_fd < 0)
  return false;

 int w = writen(new_fd, data, size);

 if(w == -1){
  printf("INCREDIBBBILE\n"); //TODO
  return false;
}


 close(new_fd);
 return true;
}

void printLastErrorMsg(){
  printf("ERROR: %s\n", last_error_msg);
  memset(last_error_msg, '\0', 256);
}


bool getResponseMsg(){
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);
  read(fd,response_buf,MAX_RESPONSE_SIZE);

  if(strncmp("OK",response_buf,2) == 0)
   return true;
  else{
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, response_buf);
  }

 return false;
}


void *getDataResponseMsg(){
  char response_buf[MAX_RESPONSE_SIZE];
  memset(response_buf, '\0', MAX_RESPONSE_SIZE);
  read(fd,response_buf,MAX_RESPONSE_SIZE);

  char* ptr = NULL;
  long int len = 0;
  char* first_str =  strtok_r(response_buf, " ", &ptr);

  if(first_str == NULL || strcmp(first_str,"KO") == 0){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "KO [Can't retrieve the object]");
    return NULL;
  }

  char* len_str =  strtok_r(NULL, " ", &ptr);
  char* newline = strtok_r(NULL, " ", &ptr);

  if(len_str != NULL) len = atol(len_str);

  if(len == 0){
    memset(last_error_msg, '\0', 256);
    strcpy(last_error_msg, "Object length is equal to zero\n");
    return NULL;
  }

  void *data = calloc(len,1);
  int b = MAX_RESPONSE_SIZE-DATA_MSG_LENGTH-getNumberOfDigits(len);

  int n;
  if(len-b > 0){
    memcpy(data,(newline+2),b);
    n = readn(fd, ((char*) data)+b,len-b);
    if(n <= 0){printf("PROBLEMA");return NULL;}  //TODO
  }
  else memcpy(data,(void*)(newline+2),len);

 return data;
}


int os_connect(char *name) {
  struct sockaddr_un serv_addr;
  int c;
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

  if( (c = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1)
    return false;

  int len = strlen(name);
  int N = len + REGISTER_LENGTH;

  char* buff = calloc(N, sizeof(char));
  sprintf(buff,"REGISTER %s \n",name);

	int n = write(fd, buff, N);
  free(buff);

  if(n < 0)return false;

	return getResponseMsg();
}


int os_store(char *name, void *block, size_t len) {
  if(block == NULL || len <= 0 )return false;

  size_t store_size = STORE_LENGTH + strlen(name) + getNumberOfDigits(len);
  char* buff = calloc(store_size+1, sizeof(char));

  sprintf(buff,"STORE %s %lu \n ",name, len);

  char* tmp = calloc(store_size+1+len,sizeof(char));
  memcpy(tmp,buff,store_size);
  memcpy(tmp+store_size,block,len);

  writen(fd,tmp,store_size+len);
	free(buff);
	free(tmp);

	return getResponseMsg();
}


void *os_retrieve(char* name){
  int N = strlen(name) + RETRIEVE_LENGTH + 1;
  char* buff = calloc(N, sizeof(char));
  sprintf(buff, "RETRIEVE %s \n", name);

  writen(fd, buff, N-1);
  free(buff);

  return getDataResponseMsg();
}

int os_delete(char* name){
  int N = strlen(name) + DELETE_LENGTH + 1;
  char* buff = calloc(N, sizeof(char));
  sprintf(buff,"DELETE %s \n", name);

  writen(fd, buff, N-1);
  free(buff);

 return getResponseMsg();
}


int os_disconnect(){
 int w = writen(fd, "LEAVE \n", 7);
 if(w == -1)return false;

 bool response = getResponseMsg();
 int c = close(fd);
 fd = -1;

 if(c == -1)return false;

 return response;
}


bool str_equals(char* a, char* b){
 if(a == NULL && b == NULL)
  return true;

 if(a == NULL || b == NULL)
  return false;

 return (strcmp(a,b) == 0);
}


bool sendFile(char* src, char* dest){
  FILE *f = fopen(src, "rb");
  bool result = false;

  if(f){
   int d = fileno(f);

   if(d == -1)return false;

   struct stat finfo;
   fstat(d, &finfo);

   size_t size = finfo.st_size;
   char *buffer = calloc(size, sizeof(char));
   fread(buffer, size, 1, f);
   result = os_store(dest,buffer, size);
   free(buffer);
   fclose(f);
 	}

	return result;
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
