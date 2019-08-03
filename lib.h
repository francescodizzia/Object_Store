#if !defined(_OBJECT_STORE)
#define _OBJECT_STORE

#define true 1
#define false 0
#define bool int

#define SOCKNAME "objstore.sock"
#define MAX_CONN 256

#define MAX_PATH_SIZE 4096
#define DATA_DIRECTORY "./data/"

#define MAX_HEADER_SIZE 128
#define MAX_RESPONSE_SIZE 128


size_t getNumberOfDigits(size_t k);
char* getUserPath(char* username);
bool createFile(char* filename, void* data, char* username, size_t size);
bool sendFile(char* src, char* dest);
bool str_equals(char* a, char* b);

int os_connect(char *name);
int os_store(char* name, void* block, size_t len);
void *os_retrieve(char* name);
int os_delete(char *name);
int os_disconnect();

int readn(long fd, void *buf, size_t size);
int writen(long fd, void *buf, size_t size);


#endif
