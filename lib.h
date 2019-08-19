#include <stdbool.h>

#if !defined(_OBJECT_STORE)
#define _OBJECT_STORE

void printLastErrorMsg();

int os_connect(char *name);
int os_store(char* name, void* block, size_t len);
void *os_retrieve(char* name);
int os_delete(char *name);
int os_disconnect();


#endif
