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
#include <pthread.h>
#include <sys/stat.h>
#include <lib.h>

bool terminate = false;


void sigIntHandler(){
 terminate = true;
 printf("ZA WARUDOOO\n");
}

void printBool(int b){
  if(b == true)
   printf("true\n");
  else
   printf("false\n");
}

int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);


  char string[] = "Prova 12345";

  os_connect("francesco");

  os_store("testo",string,strlen(string));
  sendFile("./art.gife","./goomba.gif");
  sendFile("./jojo.jpg","./JoJo.jpg");

  close(fd);
  printf("\nCLOSED!\n");

  return 0;
}
