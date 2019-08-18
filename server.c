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
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <ftw.h>

#include <thread_worker.h>
#include <hashtable.h>

#define HASH_TABLE_SIZE 256

#define ONE_MB 1000000


volatile sig_atomic_t running = true;
pthread_mutex_t client_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fs_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty;

int n_clients = 0;
hashtable HT;

size_t total_size = 0;
size_t objects = 0;
size_t folders = -1;

void cleanup() {
  unlink(SOCKNAME);
}

int setStats(const char* filename, const struct stat* stats, int type){
  if(type == FTW_F) // È un file
    objects++;
  else if(type == FTW_D) //È una directory
    folders++;

  total_size += stats->st_size;
  return 0;
}


void resetStats(){
  total_size = 0;
  objects = 0;
  folders = -1;
}

void printStats(){
  int k = ftw("./data/",setStats,0);

  if(k == -1){resetStats();return;}

  float size_in_MB = ((float)total_size)/ONE_MB;
  char string[512];
  memset(string, '\0',512);

  pthread_mutex_lock(&client_mtx);
  sprintf(string,"\n**************************************************\nSize totale degli oggetti: %lu byte (%.2f MB)\nNumero di oggetti: %lu\nCartelle: %lu\nClient connessi: %d\n**************************************************\n",total_size, size_in_MB, objects,folders,n_clients);

  write(1,string,strlen(string));
  resetStats();
  pthread_mutex_unlock(&client_mtx);
}




void spawnThreadWorker(long connfd, void *(*startFunction) (void *)) {
  pthread_attr_t thattr;
  pthread_t thid;

  if(pthread_attr_init(&thattr) != 0) {
	    fprintf(stderr, "pthread_attr_init FALLITA\n");
	    close(connfd);
	    return;
  }

    // settiamo il thread in modalità detached
  if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
	   fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
     close(connfd);
     return;
  }

  if (pthread_create(&thid, &thattr, startFunction, (void*)connfd) != 0) {
     fprintf(stderr, "pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   close(connfd);
	   return;
  }
}

void spawnSignalThread(void* ptr, void *(*startFunction) (void *)) {
  pthread_attr_t thattr;
  pthread_t thid;

  if(pthread_attr_init(&thattr) != 0) {
	    fprintf(stderr, "pthread_attr_init FALLITA\n");
	    return;
  }

  if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
	   fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
     return;
  }

  if (pthread_create(&thid, &thattr, startFunction, ptr) != 0) {
     fprintf(stderr, "pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   return;
  }
}

void* signal_handler (void* ptr) {
    sigset_t set = *((sigset_t*) ptr);

    int signal;
    while (running) {
        sigwait(&set, &signal);

        if(signal == SIGUSR1) //Stampo le stats
          printStats();
        else //Ogni altro segnale fa chiudere in modo 'gentile' il server e i client
          running = false;
    }
    printf("Sto terminando il thread dei segnali...\n");

    return NULL;
}



int main(){
  cleanup();
  atexit(cleanup);

  sigset_t set;
  sigfillset(&set);
  pthread_sigmask(SIG_SETMASK, &set, NULL);

  spawnSignalThread(&set, signal_handler);


  int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(server_fd == -1){
    fprintf(stderr, "Errore nella creazione della socket\n");
    return -1;
  }

  struct sockaddr_un sa;
  strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
  sa.sun_family = AF_UNIX;

  if(bind(server_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1){
    fprintf(stderr, "Errore nel binding\n");
  }

  if(listen(server_fd, SOMAXCONN) == -1){
    fprintf(stderr, "Errore nella listen\n");
  }

  HT = createHashTable(HASH_TABLE_SIZE);

  struct timeval timer;
  fd_set fds,r_fds;

  FD_ZERO(&fds);
  FD_SET(server_fd, &fds);

  int select_ret, connfd;

  while(running){
    timer.tv_sec = 0;
    timer.tv_usec = 10000; //10000 microsecondi sono 10 millisecondi
    r_fds = fds;

    select_ret = select(server_fd+1, &r_fds, NULL, NULL, &timer);

    if(select_ret == -1)
	    break;

     else if(select_ret > 0){
		    connfd = accept(server_fd, (struct sockaddr*)NULL ,NULL);
        spawnThreadWorker(connfd, thread_worker);
     }

  }

  pthread_mutex_lock(&client_mtx);
    if(n_clients > 0){
      printf("Attendo la terminazione degli altri thread...\n");
      pthread_cond_wait(&empty, &client_mtx);
    }
  pthread_mutex_unlock(&client_mtx);

  printf("Tutti i thread sono stati terminati con successo!\n");


  freeHashTable(&HT);
  close(server_fd);

  return 0;
}
