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


void parse_request(int c_fd, char *str){
 if(str == NULL)return;

 char *ptr = NULL;
 long int len = 0;

 char *action = strtok_r(str, " ", &ptr);
 char *name = strtok_r(NULL, " ", &ptr);
 char* len_s = strtok_r(NULL, " ", &ptr);
 //newline
 char *newline = strtok_r(NULL, " ", &ptr);

if(action == NULL)return;

if(len_s != NULL)  len = atol(len_s);

 //debug
 if(true){
  ;// printf("Action:%s[%d]|Name:%s[%ld]|len:%ld[%d]\n",action,strlen(action),name,strlen(name),len,getNumberOfDigits(len));
}

  if(str_equals(action,"REGISTER")){
    char* user_path = getUserPath(name);
    int result = mkdir(user_path,  0755);
    free(user_path);


//////////////////////////////////NO THREAD SAFE - MUST FIX! /////////////////////////////
    //if not connected then
    if(currentUser == NULL){
      //currentUser = calloc(strlen(name)+1,sizeof(char));
      //strcpy(currentUser,name);
      currentUser = strdup(name);
      printf("REGISTERED %s", currentUser);
      //currentUser = name;
    // printf("currentUser is %s\n",name);
    }else{
      printf("User %s already registered.\n",currentUser);
    }

    if(result == 0){ //Successo nella creazione della dir
      //writen(c_fd,"OK \n",MAX_RESPONSE_SIZE);
      write(c_fd,"OK \n",5);
      printf("Invio OK\n");
    }
    else{ //Fallimento
      printf("Invio FALLIMENTO\n");
      char fail_buf[MAX_RESPONSE_SIZE];
      memset(fail_buf,'\0',MAX_RESPONSE_SIZE);

      sprintf(fail_buf,"KO %d \n", errno);
      writen(c_fd,fail_buf,MAX_RESPONSE_SIZE);
    }
    printf("[%d] *fine richiesta*\n\n",c_fd);
  }

  if(str_equals(action,"STORE") ){
    printf("CIAO");
    void *data = calloc(len,1);
    int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);

   printf("letti: %d, da leggere: %d\n, letta: %s",b,len-b,(char*)data);
//    memcpy(data,newline+2,len);
    //controllo se ho altro da leggere

    int n;
    if(len-b > 0){
      memcpy(data,newline+2,b);
      n = readn(c_fd, ((char*) data)+b,len-b);
      if(n == -1){printf("PROBLEMA\n");}  //TODO
    }else{
        memcpy(data,newline+2,len);
    }
    //memcpy(data,(newline+2),len);//+2
    if(currentUser == NULL){
      printf("We got a problem here.\n");
      write(c_fd,"KO user not registered \n",MAX_RESPONSE_SIZE);
    }
    else{

      int success = createFile(name,data,currentUser,len);


      if(success){
        printf("Stored %p\n\n",data);
        //write(c_fd,"OK \n",MAX_RESPONSE_SIZE);
        writen(c_fd,"OK \n",5);
      }
      else{
        printf("Error storing %p",data);
        char err_buf[MAX_RESPONSE_SIZE];
        memset(err_buf,'\0',MAX_RESPONSE_SIZE);
        sprintf(err_buf,"KO %s (%d) \n",strerror(errno),errno);

        write(c_fd,err_buf,MAX_RESPONSE_SIZE);
      }
    }

    free(data);
  }

}
