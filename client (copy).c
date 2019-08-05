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
#include <stdbool.h>

bool terminate = false;
bool final = false;

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
 char string[] = "Prova 12345";

   bool a = os_connect(user);
   bool b = os_store("testo",string,strlen(string));
   bool c = sendFile("./art.gife","./goomba.gif");
   bool d = sendFile("./jojo.jpg","./JoJo.jpg");
   bool z = true;

/*
   if(strcmp(user,"user_88") == 0){
     void* block = os_retrieve("goomba.gif");
     os_store("copy.gif",block,261);
  }
*/

/*
   if(str_equals(user, "user_90") || str_equals(user, "user_10"))
      z = os_delete("JoJo.jpg");

   if(str_equals(user, "user_100"))
      os_delete("../user_1/JoJo.jpg");
*/
   bool e = os_disconnect();

 final = a && b && c && d && e && z;
}

int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigIntHandler);

  test0(argv[1]);

  if(final)
    printf("[OK] ");
  else
    printf("[KO] ");
  printf("%s closed.\n",argv[1]);

  return 0;
}
