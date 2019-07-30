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


char currentUser[MAX_USER_SIZE];

void addUser(char* name){
  //if NOT connected then
    memset(currentUser, '\0',MAX_USER_SIZE);
    strcpy(currentUser,name);
}



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
/*
  if(str_equals(action,"REGISTER")){
    char* user_path = getUserPath(name);
    int result = mkdir(user_path,  0755);
    free(user_path);

    //if not connected then
      addUser(name);

    if(result == 0){ //Successo nella creazione della dir
      writen(c_fd,"OK \n",MAX_RESPONSE_SIZE);
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
*/
  if(str_equals(action,"STORE") ){
    printf("CIAO\n");
    void *data = calloc(len,1);
    int b = MAX_HEADER_SIZE-10-strlen(name)-getNumberOfDigits(len);
    memcpy(data,newline+2,b);
    //printf("letti: %d, da leggere: %d\n, letta: %s",b,len-b,(char*)data);
    int n =readn(c_fd,data+b,len-b);

    //memcpy(data,(newline+2),len);//+2


    int success = createFile(name,data,"user_1",len);


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
   free(data);
  }

}
