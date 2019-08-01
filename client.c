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

void test0(char* user){
 //char users[32];
 char string[] = "Prova 12345";

 for(int i = 0; i < 1; i++){
   //sprintf(users,"user_%d",i);
   os_connect(user);

   os_store("testo",string,strlen(string));
   sendFile("./art.gife","./goomba.gif");
   sendFile("./jojo.jpg","./JoJo.jpg");

   os_disconnect();
 }

}

int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);

  test0(argv[1]);
  printf("[%s]: closed!\n",argv[1]);

  return 0;
}
