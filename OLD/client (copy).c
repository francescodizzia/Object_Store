#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lib.h"

//#define BUFSIZE 4

int main(){
  struct sockaddr_un serv_addr;

  int BUFSIZE = N;
  char buf[BUFSIZE];
  int c;

  int exit = false;

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);


  if( c = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
   printf("[-] Server not up\n");
   return -1;
 }
int b=0;
c=0;

  while(!exit){
    memset(buf, '\0', BUFSIZE);
    scanf("%s",buf);

    //b = writen(fd, buf+c, strlen(buf));
    write(fd, buf, strlen(buf));

    if(str_equals(buf, "quit"))
     exit = true;

  //  int n = read(fd, buf, 256);
  //  buf[n] = '\0';


  //  printf("result: %s\n", buf);
   c += b;

  }

  close(fd);

  return 0;
}
