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
#include <server.h>
#include <parser.h>
#include <hashtable.h>
#include <thread_worker.h>


void *thread_worker(void *arg) {
  long connfd = (long)arg;
  //fcntl(connfd, F_SETFL, O_NONBLOCK);

  sigset_t set;

  sigfillset(&set);
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  char *header = calloc(MAX_HEADER_SIZE, sizeof(char));
  char currentUser[USER_MAX_LENGTH];
  memset(currentUser,'\0',USER_MAX_LENGTH);

  pthread_mutex_lock(&client_mtx);
  n_clients++;
  pthread_mutex_unlock(&client_mtx);

  int u = 0;

  while(running){
    u = read(connfd, header, MAX_HEADER_SIZE);
    if(u <= 0)break;
    parse_request(connfd,header,currentUser);
    memset(header, '\0', MAX_HEADER_SIZE);
  }

  pthread_mutex_lock(&client_mtx);
    n_clients--;

    if(currentUser[0] != '\0'){
      removeHashTable(&HT, currentUser);
      memset(currentUser, '\0', USER_MAX_LENGTH);
    }

    if(n_clients <= 0)
      pthread_cond_signal(&empty);
  pthread_mutex_unlock(&client_mtx);


  free(header);
  close(connfd);

  return NULL;
}
