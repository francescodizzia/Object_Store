#include <stdio.h>
#include <stdlib.h>

#include <lib.c>
#include <stdbool.h>
#include <unistd.h>


int main(){

  bool a = os_connect("evil");

  sleep(5);

  bool b = os_disconnect();

  printf("ciao\n");

  if(a && b)
    printf("PASSATO\n");
  else
    printf("ROP\n");

  return 0;
}
