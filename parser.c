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


bool sendOK(int connfd){
  int w = writen(connfd,"OK \n",4);
  if(w < 0)return false;
  return true;
}

void leave(int connfd, char* currentUser){
  HT = removeHashTable(HT,currentUser);
  sendOK(connfd);
}


bool parse_request(int connfd, char *str,char* currentUser){
 if(str == NULL)return false;

 char *ptr = NULL;
 long int len = 0;

 char *action = strtok_r(str, " ", &ptr);
 if(action == NULL){return false;}
 char *name = strtok_r(NULL, " ", &ptr);
 char* len_s = strtok_r(NULL, " ", &ptr);
 //newline
 char *newline = strtok_r(NULL, " ", &ptr);


if(len_s != NULL)  len = atol(len_s);


  if(str_equals(action,"REGISTER")){
    char* user_path = getUserPath(name);
    int result = mkdir(user_path,  0755);
    free(user_path);

    printf("REGISTRO %s\n",name);

    if(currentUser[0] == '\0'){
      strcpy(currentUser,name);
      HT = insertHashTable(HT,currentUser);
    }
    else if(str_equals(currentUser,name)){
      printf("User %s already registered.\n",currentUser);
      return false; //TODO FIX
    }
    if(result == 0){ //Successo nella creazione della dir
      writen(connfd,"OK \n",4);
      return true;
    }
    else{ //FAIL
      char fail_buf[MAX_RESPONSE_SIZE];
      memset(fail_buf,'\0',MAX_RESPONSE_SIZE);

      sprintf(fail_buf,"KO %d \n", errno);
      writen(connfd,fail_buf,MAX_RESPONSE_SIZE);
      return false;
    }
  //  printf("[%d] *fine richiesta*\n\n",connfd);
  }

  else if(str_equals(action,"STORE") ){
    void *data = calloc(len,1);
    int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);

    int n;
    if(len-b > 0){
      memcpy(data,(newline+2),b);
      n = readn(connfd, ((char*) data)+b,len-b);
      if(n <= 0){printf("PROBLEMA");return false;}  //TODO
    }else memcpy(data,(void*)(newline+2),len);

    //memcpy(data,(newline+2),len);//+2
    if(currentUser[0] == '\0'){
      printf("We got a problem here.\n");
      write(connfd,"KO user not registered \n",MAX_RESPONSE_SIZE);
      return false;
    }
    else{

      int success = createFile(name,data,currentUser,len);

      if(success){
        //printf("Stored %p\n\n",data);
        //write(c_fd,"OK \n",MAX_RESPONSE_SIZE);
        writen(connfd,"OK \n",4);
        free(data);
        return true;
      }
      else{
        printf("Error storing %p",data);
        char err_buf[MAX_RESPONSE_SIZE];
        memset(err_buf,'\0',MAX_RESPONSE_SIZE);
        sprintf(err_buf,"KO %s (%d) \n",strerror(errno),errno);

        write(connfd,err_buf,MAX_RESPONSE_SIZE);
        return false;
      }

    }

    free(data);

  }else if(str_equals(action,"LEAVE")){
    leave(connfd, currentUser);
  }

  return true;
}
