#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include "lib.h"

bool terminate = false;

void sigIntHandler(){
 terminate = true;
 printf("ZA WARUDOOO\n");
}

int main(){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);

  struct sockaddr_un serv_addr;

  char buf[1024];
  int c;

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
/*  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
*/
  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);


  if( (c = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1){
   printf("[-] Server not up\n");
   return -1;
 }

 char* buf2=NULL;

  while(!terminate){
    memset(buf, '\0', 1024);
    scanf("%s",buf);

    int n = strlen(buf)+1;

    int w = writen(fd, &n, sizeof(int));
    w = writen(fd, buf, n);

   if(w <= 0)
     break;

    buf2 = calloc(n,sizeof(char));

    int r = readn(fd, &n, sizeof(int));
    r = readn(fd, buf2, n);

   if(r <= 0)
     break;


    buf2[n-1] = '\0';
    printf("result: %s\n",buf2);
    free(buf2);

    if(str_equals(buf, "quit"))
     terminate = true;
  }

  close(fd);

  return 0;
}
