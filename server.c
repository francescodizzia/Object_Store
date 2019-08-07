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

#include <lib.h>
#include <thread_worker.h>
#include <hashtable.h>


#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

#define DEBUG_ENABLED 0
#define HASH_TABLE_SIZE 256



#define DEBUG_CMD(c) \
  if(DEBUG_ENABLED) {c;}



volatile sig_atomic_t running = true;
volatile sig_atomic_t print_stats = false;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty;

sigset_t set;

int n_clients = 0;
hashtable HT;


void cleanup() {
  unlink(SOCKNAME);
}

static unsigned int total = 0;

int sum(const char *fpath, const struct stat *sb, int typeflag) {
    total += sb->st_size;
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
	    //close(connfd);
	    return;
  }

    // settiamo il thread in modalità detached
  if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
	   fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
     pthread_attr_destroy(&thattr);
    //close(connfd);
     return;
  }

  if (pthread_create(&thid, &thattr, startFunction, ptr) != 0) {
     fprintf(stderr, "pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	  // close(connfd);
	   return;
  }
}


void* signal_handler (void* ptr) {
    // Set di segnali da aspettare
    sigset_t set = *((sigset_t*) ptr);
    // Stampa un messaggio di log
    printf("[objectstore] Signal handling thread started and waiting for signals\n");
    // Entra nel loop di attesa dei segnali
    int signal;
    while (running) {
        // Attende un segnale
        sigwait(&set, &signal);
        // Riconosce il tipo di segnale

        if(signal == SIGUSR1)
          write(1,"ciao\n",5);//print_report();
        else
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
  /*sigaddset(&set, SIGPIPE);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGUSR1);
*/
  sigfillset(&set);
  pthread_sigmask(SIG_SETMASK, &set, NULL);
  pthread_t sig_handler_id;
  //pthread_create(&sig_handler_id, NULL, signal_handler, (void*) &set);
  spawn_thread2(&set, signal_handler);

 int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
 struct sockaddr_un sa;
 strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
 sa.sun_family = AF_UNIX;

 bind(server_fd, (struct sockaddr *)&sa, sizeof(sa));
 listen(server_fd, SOMAXCONN);

 int sret,connfd;

 fd_set fds, ready_fds;
 struct timeval timeout;
 timeout.tv_sec = 0;
 timeout.tv_usec = 100000;

 FD_ZERO(&fds);
 FD_SET(server_fd, &fds);

 HT = createHashTable(HASH_TABLE_SIZE);

 while(running){

   ready_fds = fds;
   sret = select(server_fd+1,&ready_fds, NULL, NULL, &timeout);

   if(sret == 0){
     //printf("timeout\n");

   }
   else{

      if(sret == -1){
       printf("OMAE WA MO SHINDEIRU\n");
       break;
     }


      connfd = accept(server_fd, (struct sockaddr*)NULL ,NULL);
      spawn_thread(connfd, thread_worker);
    }



}


  printHashTable(HT);

  pthread_mutex_lock(&mtx);
  if(n_clients > 0){
   printf("[X] WAITING FOR THE THREADS\n");
   pthread_cond_wait(&empty, &mtx);
   printf("[+] DONE\n");
  }
  pthread_mutex_unlock(&mtx);

  freeHashTable(&HT);
  close(server_fd);

  printf("[+] Tutti i thread sono stati terminati con successo!\n");

  return 0;
}
