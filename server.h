#if !defined(_SERVER)
#define _SERVER
#include <hashtable.h>

extern volatile sig_atomic_t running;
extern pthread_cond_t empty;

extern hashtable HT;
extern int n_clients;
extern pthread_mutex_t client_mtx;

#endif
