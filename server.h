#if !defined(_SERVER)
#define _SERVER


#define SYSCALL(r,c,e) \
    if((r=c)==-1) { perror(e);printf(" errore:%d\n",errno); exit(1);}

extern pthread_mutex_t mtx;
extern int n_clients;
extern volatile sig_atomic_t running;
extern pthread_cond_t empty;



#endif
