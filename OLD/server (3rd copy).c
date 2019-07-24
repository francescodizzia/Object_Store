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


#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

static volatile sig_atomic_t running = true;
static pthread_mutex_t mtx;
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
  //int flags = fcntl(connfd, F_GETFL, 0);
  //fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

  pthread_mutex_lock(&mtx);
  n_clients++;
  pthread_mutex_unlock(&mtx);

  while(running){
	 msg_t str;
   int n;
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

  close(connfd);

  pthread_mutex_lock(&mtx);
  n_clients--;
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

 fd_set set, tmpset;
     // azzero sia il master set che il set temporaneo usato per la select
     FD_ZERO(&set);
     FD_ZERO(&tmpset);

     // aggiungo il listener fd al master set
     FD_SET(fd, &set);

     // tengo traccia del file descriptor con id piu' grande
     int fdmax = fd;

 long conn_fd;
 bool termina = 0;

 do{
  /* conn_fd = accept(fd, NULL, 0);

   spawn_thread(conn_fd);
*/



 printf("%d\n",n_clients);

tmpset = set;
if (select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) {
    if (errno==EINTR) {
  //if (termina) break;
    printf("EINTR\n");
    if(!running)
      break;
    }
}
// cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
for(int i=0; i <= fdmax; i++) {
    if (FD_ISSET(i, &tmpset)) {
  long connfd;
  if (i == fd) { // e' una nuova richiesta di connessione
      connfd = accept(fd, (struct sockaddr*)NULL ,NULL);
      FD_SET(connfd, &set);
      if(connfd > fdmax) fdmax = connfd;

      spawn_thread(connfd);

      continue;
  }
  connfd = i;  // e' una nuova richiesta da un client già connesso

  //faicose

/*
  pthread_mutex_lock(&mtx);
   //printf("%d\n",n_clients);
   if(!running && n_clients == 0)
    break;
  pthread_mutex_unlock(&mtx);
*/

}
}
}while(running);


while(n_clients > 0){
  ;
}



 //close(conn_fd);
 close(fd);


  return 0;
}
