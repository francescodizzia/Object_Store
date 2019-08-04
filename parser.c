#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>

#include <lib.h>
#include <thread_worker.h>
#include <server.h>

#define target_output stdout


void sendOK(int connfd, char* currentUser, char* operation){
  writen(connfd,"OK \n",4);
  fprintf(target_output,"user: %-15sop: %-10s\tOK \n",currentUser,operation);
}


void sendKO(int connfd, char* currentUser, char* operation){
  char fail_buf[MAX_RESPONSE_SIZE];
  memset(fail_buf,'\0',MAX_RESPONSE_SIZE);
  sprintf(fail_buf,"KO %d (%s) \n", errno, strerror(errno));
  writen(connfd,fail_buf,strlen(fail_buf));
  fprintf(target_output,"user: %-15sop: %-10s\t%s",currentUser,operation,fail_buf);
}


void sendKO_custom(int connfd, char* currentUser, char* operation, char* message){
  writen(connfd,message,strlen(message));
    fprintf(target_output,"user: %-15sop: %-10s\t%s",currentUser,operation,message);
}


void leave(int connfd, char* currentUser){
  HT = removeHashTable(HT,currentUser);
  sendOK(connfd,currentUser,"LEAVE");
}


void register_(int connfd, char* currentUser, char* name){
  char* user_path = getUserPath(name);
  int result = mkdir(user_path,  0755);
  free(user_path);


  if(currentUser[0] == '\0'){
    strcpy(currentUser,name);
    HT = insertHashTable(HT,currentUser);
  }
  else if(str_equals(currentUser,name)){
    printf("User %s already registered.\n",currentUser);
    return ; //TODO FIX
  }
  if(result == 0) //Successo nella creazione della dir
    sendOK(connfd,currentUser,"REGISTER");
  else{
    if(errno == EEXIST)
      sendOK(connfd,currentUser,"REGISTER");
    else
      sendKO(connfd,currentUser,"REGISTER");
  }

}

void store(int connfd, char* currentUser ,char* name, long int len, char* newline){
  void *data = calloc(len,1);
  int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);

  int n;
  if(len-b > 0){
    memcpy(data,(newline+2),b);
    n = readn(connfd, ((char*) data)+b,len-b);
    if(n <= 0){printf("PROBLEMA");return;}  //TODO
  }
  else memcpy(data,(void*)(newline+2),len);

  if(currentUser[0] == '\0')
    sendKO_custom(connfd,currentUser,"STORE","KO User not registered \n");
  else{
    int success = createFile(name,data,currentUser,len);
    if(success)
      sendOK(connfd,currentUser,"STORE");
    else
      sendKO(connfd,currentUser,"STORE");
  }

  free(data);
}

void retrieve(int connfd, char* currentUser, char* name){
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,name);

  FILE *f = fopen(file_path, "rb");
  int d = fileno(f);

  free(user_path);
  free(file_path);

  struct stat finfo;
  fstat(d, &finfo);

  size_t size = finfo.st_size;
  void *block = calloc(size, 1);

  if(f){
   fread(block, size, 1, f);
   int N = 8 + getNumberOfDigits(size);
   char* buff = calloc(N+ 1, sizeof(char));

   sprintf(buff,"DATA %lu \n ", size);

   char* tmp = calloc(N+1+size,sizeof(char));
   memcpy(tmp,buff,N);
   memcpy(tmp+N,block,size);

   writen(connfd,tmp,N+size);
   free(buff);
   free(tmp);
 	}
  else return;

	fclose(f);
  free(block);
}


void delete(int connfd, char* currentUser, char* name){
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,name);

  bool success = (remove(file_path) == 0);

  free(user_path);
  free(file_path);

  if(success) sendOK(connfd,currentUser,"DELETE");
  else sendKO(connfd,currentUser,"DELETE");
}


void parse_request(int connfd, char *str,char* currentUser){
 if(str == NULL)return;

 char* ptr = NULL;
 long int len = 0;

 char* operation = strtok_r(str, " ", &ptr);
 if(operation == NULL)return;
 char* name = strtok_r(NULL, " ", &ptr);
 char* len_str = strtok_r(NULL, " ", &ptr);
 char* newline = strtok_r(NULL, " ", &ptr);


 if(len_str != NULL)
    len = atol(len_str);


  if(str_equals(operation, "REGISTER"))
    register_(connfd, currentUser, name);
  else if(str_equals(operation, "STORE"))
    store(connfd, currentUser, name, len, newline);
  else if(str_equals(operation, "RETRIEVE"))
    retrieve(connfd, currentUser, name);
  else if(str_equals(operation, "DELETE"))
    delete(connfd, currentUser, name);
  else if(str_equals(operation, "LEAVE"))
    leave(connfd, currentUser);


}
