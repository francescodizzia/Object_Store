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
  if(!c){\
    return false;\
  }\
}

char* makeTestArray(int N){
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
  char* test;

  ASSERT_BOOL(os_connect(user));

  for(int i = 0; i < 20; i++){
    size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    sprintf(obj_name,"object_%d",i+1);

    test = makeTestArray(size);
    ASSERT_BOOL(os_store(obj_name, test, size));
    free(test);
  }

  ASSERT_BOOL(os_disconnect());


  return true;
}

bool test2(char* user){

  ASSERT_BOOL(os_connect(user));

  void *retrieved_obj = NULL;
  char obj_name[128];
  memset(obj_name, '\0', 128);

  int size = 0;
  char *test;

  for(int i = 0; i < 20; i++){
    sprintf(obj_name,"object_%d",i+1);
    size = 100 + ((i*17)*(i*17));
    if(size > 100000)
      size = 100000;

    retrieved_obj = os_retrieve(obj_name);
    if(retrieved_obj == NULL){
      return false;
    }


    test = makeTestArray(size);
    if(memcmp(retrieved_obj, test, size) != 0){
      printf("ERROR: contents not equal\n");
      return false;
    }

    memset(obj_name, '\0', 128);
    free(retrieved_obj);
    free(test);
  }

  ASSERT_BOOL(os_disconnect());

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

  ASSERT_BOOL(os_disconnect());

  return true;
}

bool test4(char* user, char* src, char* dest){
  bool result = false;

  ASSERT_BOOL(os_connect(user));

  FILE *f = fopen(src, "rb");
  if(f){
   int d = fileno(f);

   if(d == -1)return false;

   struct stat finfo;
   if(fstat(d, &finfo) == -1)return false;

   size_t size = finfo.st_size;
   char *buffer = calloc(size, sizeof(char));
   if(!buffer) return false;
   fread(buffer, size, 1, f);
   result = os_store(dest, buffer, size);
   free(buffer);
   fclose(f);
  }
  else return false;

  ASSERT_BOOL(os_disconnect());

  return result;
}

int main(int argc,char* argv[]){

//  signal(SIGPIPE, SIG_IGN);

  if(argc < 3){
    printf("Argomenti mancanti\n");
    return -1;
  }

  char* user = strdup(argv[1]);
  int test_code = atol(argv[2]);
  char* data_src = strdup(argv[3]);
  char* dest = strdup(argv[4]);
  bool result = false;

  if(test_code == 1)
    result = test1(user);
  else if(test_code == 2)
    result = test2(user);
  else if(test_code == 3)
    result = test3(user);
  else if(test_code == 4 && data_src && dest)
    result = test4(user, data_src, dest);

  if(result)
    printf("[OK] Test %d passato con successo! [utente: %s]\n", test_code, user);
  else{
    printf("[KO] Test %d fallito... [utente: %s]\n", test_code, user);
    printLastErrorMsg();
  }

  free(user);
  if(data_src) free(data_src);
  if(dest) free(dest);
  return 0;
}
