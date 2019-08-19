#if !defined(_SHARED)
#define _SHARED
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

#define SOCKNAME "objstore.sock"

#define MAX_PATH_SIZE 4096
#define DATA_DIRECTORY "./data/"

#define MAX_HEADER_SIZE 512
#define MAX_RESPONSE_SIZE 128


extern bool createFile(char* filename, void* data, char* username, size_t size);
extern char* getUserPath(char* username);
extern size_t getNumberOfDigits(size_t k);
extern int writen(long connfd, void* buffer, size_t size);
extern int readn(long connfd, void* buffer, size_t size);
extern bool str_equals(char* a, char* b);

#endif
