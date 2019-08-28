#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>

#if !defined(_COMMON)
#define _COMMON

//Nome della socket
#define SOCKNAME "objstore.sock"

//Massima grandezza di un path su UNIX
#define MAX_PATH_SIZE 4096
#define DATA_DIRECTORY "./data/"

//Massima grandezza dell'header di default
#define MAX_HEADER_SIZE 512

//Massima grandezza della risposta
#define MAX_RESPONSE_SIZE 128

//Lunghezza base dei vari header (inclusi spazi e '\n')
#define REGISTER_LENGTH 11
#define STORE_LENGTH 10
#define RETRIEVE_LENGTH 11
#define DELETE_LENGTH 9
#define DATA_MSG_LENGTH 8


bool createFile(char* filename, void* data, char* username, size_t size);
char* getUserPath(char* username);
size_t getNumberOfDigits(size_t k);
int writen(long connfd, void* buffer, size_t size);
int readn(long connfd, void* buffer, size_t size);
bool str_equals(char* a, char* b);

#endif
