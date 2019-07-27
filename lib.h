
#if !defined(_SOCKET)
#define _SOCKET

#define true 1
#define false 0
#define bool int

extern int fd;

#define SOCKNAME "objstore.sock"
#define MAX_CONN 256

#define DEFAULT_CHUNK_SIZE 16
#define MAX_HEADER_SIZE 16
#define BUFFSIZE 16

#define MAX_RESPONSE_SIZE 64


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
