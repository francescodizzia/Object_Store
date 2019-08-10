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


#define ASSERT_BOOL(x,c){\
  x = c;\
  if(!x){\
    return false;\
  }\
}

char* makeTestArray1(int N){
  char *A = calloc(N, 1);

    int j = 0;
    for(int i = 0; i < N; i++, j++){
        if(j > 9) j = 0;
        A[i] = '0' + j;
    }

    return A;
}

bool test1(char* user){
  char obj_name[128];
  memset(obj_name,'\0',128);

  int size = 0;
  bool connected,stored,disconnected;

  ASSERT_BOOL(connected, os_connect(user));

  for(int i = 0; i < 20; i++){
    size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    sprintf(obj_name,"object_%d",i+1);

    ASSERT_BOOL(stored, os_store(obj_name, makeTestArray1(size), size));
  }

  ASSERT_BOOL(disconnected, os_disconnect());

  return true;
}

bool test2(char* user){
  bool connected,disconnected;

  ASSERT_BOOL(connected, os_connect(user));

  void *retrieved_obj = NULL;
  char obj_name[128];
  memset(obj_name, '\0', 128);

  int size = 0;

  for(int i = 0; i < 20; i++){
    sprintf(obj_name,"object_%d",i+1);
    size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    retrieved_obj = os_retrieve(obj_name);
    if(retrieved_obj == NULL){
      return false;
    }

    if(memcmp(retrieved_obj, makeTestArray1(size), size) != 0){
      printf("ERROR: contents not equal\n");
      return false;
    }

    memset(obj_name, '\0', 128);
  }

  ASSERT_BOOL(disconnected, os_disconnect());

  return true;
}

bool test3(char* user){
  char obj_name[128];
  memset(obj_name, '\0', 128);
  bool connected,deleted,disconnected;


  ASSERT_BOOL(connected, os_connect(user));

  for(int i = 0; i < 20; i++){
    sprintf(obj_name,"object_%d",i+1);
    ASSERT_BOOL(deleted, os_delete(obj_name));
    memset(obj_name, '\0', 128);
  }

  ASSERT_BOOL(disconnected, os_disconnect());

  return true;
}


int main(int argc,char* argv[]){

  signal(SIGPIPE, SIG_IGN);

  if(argc < 3){
    printf("usage\n"); 
    return -1;
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
    printf("[OK] Test %d passato con successo! [utente: %s]\n", test_code, user);
  else{
    printf("[KO] Test %d fallito... [utente: %s]\n", test_code, user);
    printLastErrorMsg();
  }

  return 0;
}
