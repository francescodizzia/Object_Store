
#if !defined(_SOCKET)
#define _SOCKET

#define true 1
#define false 0
#define bool int

extern int fd;

#define SOCKNAME "objstore.sock"
#define MAX_CONN 256
//#define N 512
#define MAX_HEADER_SIZE 512
#define BUFFSIZE 512

#define MAX_RESPONSE_SIZE 128


#define ASSERT_NULL(a,e){\
  if(a == NULL){\
   perror(e);\
   printf("%s\n",e);\
   return false;\
  }\
}


int os_store (char* name, void* block, size_t len);
bool str_equals(char* a, char* b);
int readn(long fd, void *buf, size_t size);
int writen(long fd, void *buf, size_t size);
int os_connect(char *name);

#endif // _SOCKET
