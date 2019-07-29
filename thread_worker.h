#if !defined(_THREAD_WORKER)
#define _THREAD_WORKER

void addUser(char* name);
void *thread_worker(void *arg);

#endif
