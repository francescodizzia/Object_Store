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

#include <lib.h>

bool terminate = false;


void sigIntHandler(){
 terminate = true;
 printf("ZA WARUDOOO\n");
}


int main(){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);

  struct sockaddr_un serv_addr;

  char buf[MAX_HEADER_SIZE];
  int c;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);


  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);


  if( (c = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) == -1){
   printf("[-] Server not up\n");
   return -1;
 }


  while(!terminate){
    memset(buf, '\0', MAX_HEADER_SIZE);
    fgets(buf, MAX_HEADER_SIZE, stdin);

    os_store("francesco",buf, strlen(buf));


    if(str_equals(buf, "quit"))
     terminate = true;
  }

  close(fd);

  return 0;
}
