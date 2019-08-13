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
  fprintf(target_output,"user %-15s fd: %-10dop: %-10s\tOK \n",  currentUser, connfd, operation);
}

void sendKO(int connfd, char* currentUser, char* operation, char* message){
  char* toPrint = NULL;

  if(message == NULL)
    toPrint = strerror(errno);
  else toPrint = message;

  char fail_buf[MAX_RESPONSE_SIZE];
  memset(fail_buf,'\0',MAX_RESPONSE_SIZE);

  sprintf(fail_buf,"KO [%s] \n", toPrint);
  writen(connfd,fail_buf,strlen(fail_buf));
  fprintf(target_output,"user %-15s fd: %-10dop: %-10s\t%s",currentUser, connfd,operation,fail_buf);
}


void leave(int connfd, char* currentUser){
  removeHashTable(&HT,currentUser);
  memset(currentUser, '\0', USER_MAX_LENGTH);
  sendOK(connfd, currentUser, "LEAVE");
}


void register_(int connfd, char* currentUser, char* name){
  if(isInHashTable(HT,name)){
    sendKO(connfd,name,"REGISTER","Multiple clients with the same username");
    return;
  }

  if(currentUser[0] == '\0'){
    strcpy(currentUser,name);
    insertHashTable(&HT,currentUser);
  }

  char* user_path = getUserPath(name);

    int result = mkdir(user_path,  0755);
    free(user_path);

    if(result == 0) //Successo nella creazione della dir
      sendOK(connfd, currentUser, "REGISTER");
    else{
      if(errno == EEXIST) //La directory esiste gia', ma non e' un problema
        sendOK(connfd, currentUser, "REGISTER");
      else
        sendKO(connfd, currentUser, "REGISTER", NULL);
    }

}

void store(int connfd, char* currentUser ,char* name, long int len, char* newline){
  void *data = calloc(len,1);
  int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);

  int n;
  if(len-b > 0){
    memcpy(data,(newline+2),b);
    n = readn(connfd, ((char*) data)+b,len-b);
    if(n <= 0) return;
  }
  else memcpy(data,(void*)(newline+2),len);

  if(currentUser[0] == '\0')
    sendKO(connfd, currentUser, "STORE", "User not registered");
  else{
      if(createFile(name,data,currentUser,len))
        sendOK(connfd, currentUser, "STORE");
      else
        sendKO(connfd, currentUser, "STORE", NULL);
  }

  free(data);
}

void retrieve(int connfd, char* currentUser, char* name){

  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,name);

  FILE *f = fopen(file_path, "rb");

  if(f){
    int d = fileno(f);

    struct stat finfo;
    fstat(d, &finfo);

    size_t size = finfo.st_size;
    void *block = calloc(size, 1);

    fread(block, size, 1, f);

    int N = 8 + getNumberOfDigits(size);
    char* buff = calloc(N + 1, sizeof(char));

    sprintf(buff,"DATA %lu \n ", size);

    char* tmp = calloc(N+1+size,sizeof(char));
    memcpy(tmp,buff,N);
    memcpy(tmp+N,block,size);

    writen(connfd,tmp,N+size);
    fprintf(target_output,"user %-15s fd: %-10dop: %-10s\tOK \n",  currentUser, connfd, "RETRIEVE");

    free(buff);
    free(tmp);
    free(block);
    fclose(f);
 	  }
    else
      sendKO(connfd, currentUser, "RETRIEVE", "Can't retrieve the object");

  free(user_path);
  free(file_path);

}


void delete(int connfd, char* currentUser, char* obj_name){
  char* user_path = getUserPath(currentUser);
  char* file_path = calloc(strlen(user_path)+strlen(obj_name)+1 ,sizeof(char));
  strcat(file_path,user_path);
  strcat(file_path,obj_name);

  bool success = (remove(file_path) == 0);

  if(success) sendOK(connfd, currentUser, "DELETE");
  else sendKO(connfd, currentUser, "DELETE", NULL);

  free(user_path);
  free(file_path);
}


void parse_request(int connfd, char *str, char* currentUser){
 if(str == NULL || str[0] == '\0')return;

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
