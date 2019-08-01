#include <hashtable.h>

#if !defined(_SERVER)
#define _SERVER

extern hashtable HT;

extern pthread_mutex_t mtx;
extern int n_clients;
extern volatile sig_atomic_t running;
extern pthread_cond_t empty;



#endif
