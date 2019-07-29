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

void parse_request(int c_fd, char *str){
 if(str == NULL)return;

 char *ptr = NULL;
 size_t len = 0;

 char *action = strtok_r(str, " ", &ptr);
 char *name = strtok_r(NULL, " ", &ptr);
 char* len_s = strtok_r(NULL, " ", &ptr);
 //newline
 strtok_r(NULL, " ", &ptr);
 char* data = strtok_r(NULL, " ", &ptr);

 if(len_s != NULL)
  len = atol(len_s);


 if(action == NULL)return ;

 //debug
 if(false)printf("len: %ld\n",len);

  if(str_equals(action,"REGISTER") && name != NULL){
  //DEBUG_CMD(printf("REGISTER\n"));
  chdir(DATA_DIRECTORY);
  int result = mkdir(name,  0755);

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
  else if(str_equals(action,"STORE") ){
    if(createFile(data,"fdizzia")){
    printf("Stored %s\n\n",data);
    write(c_fd,"OK \n",MAX_RESPONSE_SIZE);
    }
    else{
      printf("Error storing %s",data);
      char err_buf[MAX_RESPONSE_SIZE];
      memset(err_buf,'\0',MAX_RESPONSE_SIZE);
      sprintf(err_buf,"KO %s (%d) \n",strerror(errno),errno);

      write(c_fd,err_buf,MAX_RESPONSE_SIZE);
    }

  }

}
