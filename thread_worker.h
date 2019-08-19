#if !defined(_THREAD_WORKER)
#define _THREAD_WORKER

#include <stdbool.h>

#define USER_MAX_LENGTH 255

void *thread_worker(void *arg);

#endif
