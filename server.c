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
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>

#include "lib.h"

#define _POSIX_C_SOURCE 200809L
#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

static volatile sig_atomic_t running = true;
static pthread_mutex_t mtx;
static pthread_cond_t empty;

static int n_clients = 0;

typedef struct msg {
    int len;
    char *str;
} msg_t;


void cleanup() {
  unlink(SOCKNAME);
}

void sigIntHandler(){
 running = false;
 printf("ZA WARUDOOO\n");
}

void toup(char *str) {
    char *p = str;
    while(*p != '\0') {
        *p = (islower(*p)?toupper(*p):*p);
	++p;
    }
}

void *threadF(void *arg) {
  long connfd = (long)arg;
  int sret;

  fd_set readfds;
  struct timeval timeout;

  pthread_mutex_lock(&mtx);
  n_clients++;
  pthread_mutex_unlock(&mtx);

  while(running){
	 msg_t str;
   int n;

   FD_ZERO(&readfds);
   FD_SET(connfd, &readfds);

   timeout.tv_sec = 0;
   timeout.tv_usec = 100000;

   sret = select(connfd+1,&readfds, NULL, NULL, &timeout);

   if(sret == 0){
  /*   printf("sret = %d\n", sret);
     printf(" timeout\n")*/;
   }
   else{

   n = readn(connfd, &str.len, sizeof(int));

	 if(n==0) break;
	 str.str = calloc((str.len), sizeof(char));

   if(!str.str) {
	    perror("calloc");
	    fprintf(stderr, "Memoria esaurita....\n");
	    break;
	 }
	n = readn(connfd, str.str, str.len * sizeof(char));

	toup(str.str);


	n = writen(connfd, &str.len, sizeof(int));
	n = writen(connfd, str.str, str.len*sizeof(char));
	free(str.str);
  }
}
  close(connfd);

  pthread_mutex_lock(&mtx);
  n_clients--;
  if(n_clients <= 0)
  pthread_cond_signal(&empty);
  pthread_mutex_unlock(&mtx);

  return NULL;
}

void spawn_thread(long connfd) {
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

    if (pthread_create(&thid, &thattr, threadF, (void*)connfd) != 0) {
     fprintf(stderr, "pthread_create FALLITA");
	   pthread_attr_destroy(&thattr);
	   close(connfd);
	   return;
    }
}

int main(){
  cleanup();
  atexit(cleanup);

 //Ignoro SIGPIPE
 signal(SIGPIPE, SIG_IGN);
 signal(SIGINT, sigIntHandler);

 int fd = socket(AF_UNIX, SOCK_STREAM, 0);

 struct sockaddr_un sa;
 strncpy(sa.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
 sa.sun_family = AF_UNIX;

 bind(fd, (struct sockaddr *)&sa, sizeof(sa));
 listen(fd, MAX_CONN);

 int sret,connfd;

 fd_set readfds;
 struct timeval timeout;

 int clients = n_clients;

 while(running){

   pthread_mutex_lock(&mtx);
    if(clients != n_clients)
     printf("%d\n",n_clients);
   clients = n_clients;
   pthread_mutex_unlock(&mtx);

   FD_ZERO(&readfds);
   FD_SET(fd, &readfds);

   timeout.tv_sec = 0;
   timeout.tv_usec = 100000;

   sret = select(fd+1,&readfds, NULL, NULL, &timeout);

   if(sret == 0){
    // printf("sret = %d\n", sret);
    // printf(" timeout\n")
    ;
   }
   else{
      if(sret == -1){
       printf("OMAE WA MO SHINDEIRU\n");
       break;
      }

      connfd = accept(fd, (struct sockaddr*)NULL ,NULL);
      spawn_thread(connfd);
    }



}


/*
while(n_clients > 0){
  ; // BUSY WAITING! MALE
}
*/


  pthread_mutex_lock(&mtx);
  if(n_clients > 0){
   printf("[X] WAITING FOR CLIENTS\n");
   pthread_cond_wait(&empty, &mtx);
   printf("[+] DONE\n");
  }
  pthread_mutex_unlock(&mtx);


printf("[+] Tutti i thread sono stati terminati con successo!\n");

 //close(conn_fd);
close(fd);


return 0;
}
