#if !defined(_SOCKET)
#define _SOCKET

#define true 1
#define false 0
#define bool int

extern int fd;

#define SOCKNAME "objstore.sock"
#define MAX_CONN 256

#define MAX_PATH_SIZE 4096
#define DATA_DIRECTORY "./data/"

#define DEFAULT_CHUNK_SIZE 16
#define MAX_HEADER_SIZE 16

#define MAX_RESPONSE_SIZE 64
#define MAX_USER_SIZE 256


#define ASSERT_NULL(a,e){\
  if(a == NULL){\
   perror(e);\
   printf("%s\n",e);\
   return false;\
  }\
}

char* getUserPath(char* username);
bool createFile(char* filename, char* data, char* username);
bool str_equals(char* a, char* b);

int os_connect(char *name);
int os_store(char* name, void* block, size_t len);
int os_delete(char *name);

int readn(long fd, void *buf, size_t size);
int writen(long fd, void *buf, size_t size);


#endif // _SOCKET
