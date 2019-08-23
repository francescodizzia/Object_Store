#include <stdbool.h>

#if !defined(_THREAD_WORKER)
#define _THREAD_WORKER

#define USER_MAX_LENGTH 255

void setBlockingFD(int connfd, bool blocking);
void *thread_worker(void *arg);

#endif
