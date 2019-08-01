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

void sendFile(char* src, char* dest){
  FILE *f;

  f = fopen(src, "rb");
  int d = fileno(f);

  struct stat finfo;
  fstat(d, &finfo);

  size_t size = finfo.st_size;

  char *buffer = calloc(size,1);
  if(f) fread(buffer, size, 1, f);
  else return;
    fclose(f);
//    buffer[strlen(buffer)-1] = '\0';
  printf("\nBAKANA %ld\n",size);

  os_store(dest,buffer, size);
  free(buffer);

  //createFile("test",buffer,"");
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

  char string[] = "Prova 123456789012345678901234567890_";

  bool connected = os_connect("francesco");

 sendFile("./Object1","./new.mp4");
//  bool success = os_store("final",string,strlen(string)+1);

//  printBool(connected && success);

//  printf("[pid: %d] ho ricevuto: %s\n",(int) getpid(),response_buf);

  close(fd);
  printf("\nCLOSED!");

  return 0;
}
