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

#include <lib.h>
#include <thread_worker.h>
#include <hashtable.h>


#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

#define DEBUG_ENABLED 0
#define HASH_TABLE_SIZE 256

#define ONE_MB 1000000





volatile sig_atomic_t running = true;
volatile sig_atomic_t print_stats = false;
pthread_mutex_t client_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fs_mtx = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t tree_walk_cond;
pthread_cond_t empty;

sigset_t set;

int n_clients = 0;
hashtable HT;


void cleanup() {
  unlink(SOCKNAME);
}

size_t total_size = 0;
size_t number_objects = 0;
size_t number_users = -1;


int setStats(const char* filename, const struct stat* stats, int type){

  if(type == FTW_F) // E' un file
    number_objects++;
  else if(type == FTW_D) //E' una directory
    number_users++;

  total_size += stats->st_size;
  return 0;
}



void spawn_thread(long connfd, void *(*startFunction) (void *)) {
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

void spawn_thread2(void* ptr, void *(*startFunction) (void *)) {
  pthread_attr_t thattr;
  pthread_t thid;

  if(pthread_attr_init(&thattr) != 0) {
	    fprintf(stderr, "pthread_attr_init FALLITA\n");
	    return;
  }

    // settiamo il thread in modalità detached
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

void resetStats(){
  total_size = 0;
  number_objects = 0;
  number_users = -1;
}

void printStats(){

  pthread_mutex_lock(&fs_mtx);
    ftw("./data/",setStats,0);
  pthread_mutex_unlock(&fs_mtx);

  float size_in_MB = ((float)total_size)/ONE_MB;

  pthread_mutex_lock(&client_mtx);
    for(int i = 0; i < 50; i++)printf("*");
    printf("\nSize totale degli oggetti: %lu byte (%.2f MB)\nNumero di oggetti: %lu\nCartelle: %lu\nClient connessi: %d\n",total_size, size_in_MB, number_objects,number_users,n_clients);
    for(int i = 0; i < 50; i++)printf("*");
    printf("\n");
  pthread_mutex_unlock(&client_mtx);

  resetStats();


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
    printf("[objectstore] Signal handling thread stopped\n");

    return NULL;
}


int main(){
  cleanup();
  atexit(cleanup);

  sigset_t set;
  sigemptyset(&set);
  sigfillset(&set);
  pthread_sigmask(SIG_SETMASK, &set, NULL);

  spawn_thread2(&set, signal_handler);

 int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
 struct sockaddr_un sa;
 strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
 sa.sun_family = AF_UNIX;

 bind(server_fd, (struct sockaddr *)&sa, sizeof(sa));
 listen(server_fd, SOMAXCONN);

 int sret,connfd;

 fd_set fds, ready_fds;

 FD_ZERO(&fds);
 FD_ZERO(&ready_fds);
 FD_SET(server_fd, &fds);

 HT = createHashTable(HASH_TABLE_SIZE);

 struct timeval timeout;
 timeout.tv_sec = 0;      //0 secondi
 timeout.tv_usec = 10000; //(10000 microsecondi = 10 millisecondi)

 while(running){

   ready_fds = fds;
   sret = select(server_fd+1, &ready_fds, NULL, NULL, &timeout);
/*
   if(sret == -1)
     break; //TODO errore da gestire
   else if(FD_ISSET(server_fd ,&ready_fds)){
     connfd = accept(server_fd, NULL, NULL);
     spawn_thread(connfd, thread_worker);
   }
*/

if(sret == 0){
   //printf(" timeout\n");
 }
 else{
    if(sret == -1)
     break;

    connfd = accept(server_fd, NULL ,NULL);
    spawn_thread(connfd, thread_worker);
  }


 }

  printHashTable(HT);

  pthread_mutex_lock(&client_mtx);
    if(n_clients > 0){
      printf("[X] WAITING FOR THREADS\n");
    pthread_cond_wait(&empty, &client_mtx);
    printf("[+] DONE\n");
  }
  pthread_mutex_unlock(&client_mtx);

  freeHashTable(&HT);
  close(server_fd);

  printf("[+] Tutti i thread sono stati terminati con successo!\n");

  return 0;
}
