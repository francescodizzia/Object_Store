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


#define ASSERT_BOOL(c){\
  if(!c)\
    return false;\
}

char* makeTestArray1(int N){
  char *A = calloc(N, 1);

  //if(type == 1){
    int j = 0;
    for(int i = 0; i < N; i++, j++){
        if(j > 9) j = 0;
        A[i] = '0' + j;
    }
    return A;
  //}



}

bool test1(char* user){
  char obj_name[128];
  memset(obj_name,'\0',128);


  for(int i = 0; i < 20; i++){
    int size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    sprintf(obj_name,"object_%d",i+1);

    ASSERT_BOOL(os_connect(user));
    ASSERT_BOOL(os_store(obj_name, makeTestArray1(size), size));
    ASSERT_BOOL(os_disconnect());

  }


  return true;
}

void *getBuffer(char* src){
  FILE *f = fopen(src, "rb");
  bool result = false;

  if(f){
   int d = fileno(f);

   if(d == -1)return false;

   struct stat finfo;
   fstat(d, &finfo);

   size_t size = finfo.st_size;
   void *buffer = calloc(size, 1);
   fread(buffer, size, 1, f);

   fclose(f);
   return buffer;
  }

  printf("NULL\n");
 return NULL;
}


bool test2(char* user){
  void *f1 = getBuffer("./data/user_1/object_2");
  void *f2 = getBuffer("./data/user_1/object_3");

  if(memcmp(f1, f2, 256) == 0)
    printf("MATCH\n");
  else printf("MISMATCH\n");

  free(f1);
  free(f2);

  return true;
}

bool test3(char* user){
  char obj_name[128];
  memset(obj_name, '\0', 128);

  ASSERT_BOOL(os_connect(user));

  for(int i = 0; i < 20; i++){
    sprintf(obj_name,"object_%d",i+1);
    ASSERT_BOOL(os_delete(obj_name));
    memset(obj_name, '\0', 128);
  }


  return true;
}

int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);
  //signal(SIGINT, sigIntHandler);

  if(argc < 3){
    printf("usage\n");  //TODO
    return 1;
  }

  char* user = strdup(argv[1]);
  int test_code = atol(argv[2]);
  bool result = false;

  if(test_code == 1)
    result = test1(user);
  else if(test_code == 2)
    result = test2(user);
  else if(test_code == 3)
    result = test3(user);

  if(result)
    printf("Test %d passato con successo! [utente: %s]\n", test_code, user);
  else
    printf("Test %d fallito... [utente: %s]\n", test_code, user);

  return 0;
}
