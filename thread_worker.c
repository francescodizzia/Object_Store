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


void *thread_worker(void *arg) {
  long connfd = (long)arg;

 //MUST CHANGE!

  /*char header[TEST_SIZE];

  memset(header, '\0', TEST_SIZE);
*/
  char *header = calloc(MAX_HEADER_SIZE,sizeof(char));

  pthread_mutex_lock(&mtx);
  n_clients++;
  pthread_mutex_unlock(&mtx);

 int u=0;

  while(running){


  u = readn(connfd, header, MAX_HEADER_SIZE);

  if(u == -1)
   printf("ohhh shit here we go again\n");

  if(u == 0)break;


 mkdir("./data/user_1/",  0755);
 parse_request(connfd,header);



  }

  free(header);
  close(connfd);

  pthread_mutex_lock(&mtx);
  n_clients--;

  if(n_clients <= 0)
    pthread_cond_signal(&empty);

  pthread_mutex_unlock(&mtx);

  return NULL;
}
