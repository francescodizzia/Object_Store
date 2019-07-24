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
#include <pthread.h>

#include "lib.h"


#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

static volatile sig_atomic_t running = true;



void cleanup() {
  unlink(SOCKNAME);
}

void sigIntHandler(){
 running = false;
 printf("ZA WARUDOOO\n");
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

 int fd_client = accept(fd, NULL, 0);

 char* buf = calloc(N,sizeof(char));
 int b = 0;
 int c = 0;

 while(running){

   b = readn(fd_client, buf+c, BYTES_TO_READ);

   if(b == 0)
    break;

   printf("Ho ricevuto: %s\n",buf);


   if(b > 0 && str_equals(buf,"quit"))
    break;

    c += b;
  }


 free(buf);
 close(fd_client);
 close(fd);


  return 0;
}



/*
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

#include "lib.h"


#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

bool running = true;

void parse_request(char* header){
 //char action[MAX_ACTION_LENGTH],name[MAX_NAME_LENGTH];
 char* action = malloc(MAX_ACTION_LENGTH);
 char* name = malloc(MAX_NAME_LENGTH);

 size_t len;
 sscanf(header, "%s %s %lu \n", action, name, &len);
 printf("Action: %s   Name: %s  len: %lu\n",action, name, len);
}



void cleanup() {
  unlink(SOCKNAME);
}

void sigIntHandler(){
 running = false;
 printf("ZA WARUDOOO\n");
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

 int fd_client = accept(fd, NULL, 0);

 char* buf = calloc(N,sizeof(char));
 int b = 0;
 int c = 0;

 while(running){
   //int b = readn(fd_client, buf, BYTES_TO_READ);
   b = readn(fd_client, buf+c, BYTES_TO_READ);

   if(b == 0)
    break;

   printf("Ho ricevuto: %s\n",buf);
   //write(fd_client, "Ho ricevuto",strlen("Ho ricevuto"));

  //parse_request(buf);


   if(b > 0 && str_equals(buf,"quit"))
    break;

    c += b;
  }


 free(buf);
 close(fd_client);
 close(fd);


  return 0;
}
*/
