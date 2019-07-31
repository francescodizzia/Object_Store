#if !defined(_THREAD_WORKER)
#define _THREAD_WORKER

extern char* currentUser;
void *thread_worker(void *arg);

#endif
