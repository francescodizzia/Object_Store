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

#define TEST_SIZE 4096

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

  while(running){
    int u = 0;

    SYSCALL(u,read(connfd, header, DEFAULT_CHUNK_SIZE),"read_x1");
    if(u == 0)break;
    strcat(finalheader,header);

    if(header[u-1] == '\0'){
      parse_request(connfd, finalheader);
      memset(finalheader,'\0',TEST_SIZE);
      memset(header,'\0',TEST_SIZE);
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
