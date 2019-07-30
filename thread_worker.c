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

#define TEST_SIZE 80960

void *thread_worker(void *arg) {
  long connfd = (long)arg;

 //MUST CHANGE!
  char header[TEST_SIZE];
  char finalheader[TEST_SIZE];

  memset(header, '\0', TEST_SIZE);
  memset(finalheader, '\0', TEST_SIZE);

  pthread_mutex_lock(&mtx);
  n_clients++;
  pthread_mutex_unlock(&mtx);

 int u=0,t,sret;
 bool finished = false;

 fd_set readfds2;
 struct timeval timeout;

  while(running){

 // while( (u = read(connfd, header+u, DEFAULT_CHUNK_SIZE)) == DEFAULT_CHUNK_SIZE );
  u = read(connfd, header, TEST_SIZE);

  if(u == 0)break;
   t+=u;

 if(!finished){
   mkdir("./data/user_1/",  0755);
  //createFile("test",header,"user_1");
  parse_request(connfd,header);
  finished = true;
 }


  //  }

  }

  close(connfd);

  pthread_mutex_lock(&mtx);
  n_clients--;

  if(n_clients <= 0)
    pthread_cond_signal(&empty);

  pthread_mutex_unlock(&mtx);

  return NULL;
}
